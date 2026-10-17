// us_hydrodyn_perceive_somo.cpp -- see header. Qt/SOMO adapter around the Qt-free perception core.
#include "us_hydrodyn_perceive_somo.h"
#include "us_hydrodyn_perceive_elements.h"

#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

namespace somo_perceive {

std::vector<InAtom> from_pdb_model( const PDB_model & model ) {
    std::vector<InAtom> out;
    for ( unsigned int c = 0; c < model.molecule.size(); ++c ) {
        const PDB_chain & ch = model.molecule[ c ];
        for ( unsigned int a = 0; a < ch.atom.size(); ++a ) {
            const PDB_atom & pa = ch.atom[ a ];
            // Skip alternate locations other than the primary, as the perceiver expects one
            // position per atom; a duplicated altloc would be perceived as a bonded neighbour.
            if ( !pa.altLoc.isEmpty() && pa.altLoc != " " && pa.altLoc != "A" ) {
                continue;
            }
            InAtom x;
            // The element column is authoritative. When it is empty the element has to be
            // inferred from the atom name, which norm_element() cannot do -- see
            // element_from_atom_name(). Getting this wrong is silent: every increment becomes
            // zero and the psv comes out near zero rather than failing.
            x.element = pa.element.trimmed().isEmpty()
                ? somo_perceive::element_from_atom_name( pa.name.trimmed().toStdString(),
                                                         pa.resName.trimmed().toStdString() )
                : norm_element( pa.element.trimmed().toStdString() );
            x.x       = pa.coordinate.axis[ 0 ];
            x.y       = pa.coordinate.axis[ 1 ];
            x.z       = pa.coordinate.axis[ 2 ];
            x.serial  = (int) pa.serial;
            x.name    = pa.name.trimmed().toStdString();
            x.resName = pa.resName.trimmed().toStdString();
            x.chain   = ch.chainID.trimmed().toStdString();
            x.resSeq  = pa.resSeq.trimmed().toInt();
            out.push_back( x );
        }
    }
    return out;
}

std::map<QString,QString> hetnam_names( const QString & pdb_filename ) {
    std::map<QString,QString> out;
    QFile f( pdb_filename );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        return out;
    }
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
        QString l = ts.readLine();
        if ( !l.startsWith( "HETNAM" ) || l.length() < 16 ) {
            continue;
        }
        QString id  = l.mid( 11, 3 ).trimmed();
        QString txt = l.mid( 15 ).trimmed();
        if ( id.isEmpty() || txt.isEmpty() ) {
            continue;
        }
        if ( out.count( id ) ) {
            out[ id ] += " " + txt;      // continuation record
        } else {
            out[ id ]  = txt;
        }
    }
    f.close();
    return out;
}

std::vector< std::pair< int, int > > conect_bonds( const QString & pdb_filename,
                                                   const std::vector<InAtom> & atoms ) {
    std::vector< std::pair< int, int > > out;
    QFile f( pdb_filename );
    if ( !f.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        return out;
    }
    std::map<int,int> ser2idx;
    for ( int i = 0; i < (int) atoms.size(); ++i ) {
        ser2idx[ atoms[ i ].serial ] = i;
    }
    QTextStream ts( &f );
    while ( !ts.atEnd() ) {
        QString l = ts.readLine();
        if ( !l.startsWith( "CONECT" ) || l.length() < 11 ) {
            continue;
        }
        bool ok = false;
        int origin = l.mid( 6, 5 ).trimmed().toInt( &ok );
        if ( !ok ) {
            continue;
        }
        auto io = ser2idx.find( origin );
        if ( io == ser2idx.end() ) {
            continue;
        }
        for ( int k = 0; k < 4; ++k ) {
            int pos = 11 + 5 * k;
            if ( l.length() < pos + 1 ) {
                break;
            }
            bool pok = false;
            int partner = l.mid( pos, 5 ).trimmed().toInt( &pok );
            if ( !pok ) {
                continue;
            }
            auto ip = ser2idx.find( partner );
            if ( ip != ser2idx.end() ) {
                out.push_back( std::make_pair( io->second, ip->second ) );
            }
        }
    }
    f.close();
    return out;
}

QList<Tentative> perceive_unknown( const PDB_model         & model,
                                   const HybridTable       & tbl,
                                   const std::set<QString> & to_perceive,
                                   const QString           & pdb_filename,
                                   const somo_hydration::Table * hyd,
                                   const somo_residue_builder::Options & opt ) {
    QList<Tentative> result;

    std::vector<InAtom> atoms = from_pdb_model( model );
    if ( atoms.empty() ) {
        return result;
    }

    std::vector< std::pair< int, int > > ebonds;
    std::map<QString,QString>            hetnam;
    if ( !pdb_filename.isEmpty() ) {
        ebonds = conect_bonds( pdb_filename, atoms );
        hetnam = hetnam_names( pdb_filename );
    }

    Perceiver perc( tbl );
    Bonds bonds;
    std::vector<OutAtom> out = perc.perceive( atoms, bonds, ebonds );

    // SOMO names each instance of a non-coded residue "<RESNAME>_NC<n>". Strip that back to the
    // base code so one tentative entry covers every instance of the same chemistry.
    QRegularExpression rx_nc( "_NC\\d+$" );

    // Count instances per base name, and remember the first instance to perceive.
    std::map<QString,int> instance_count;
    for ( size_t i = 0; i < atoms.size(); ++i ) {
        QString rn = QString::fromStdString( atoms[ i ].resName );
        if ( rn.isEmpty() || !to_perceive.count( rn ) ) continue;
        QString base = rn; base.remove( rx_nc );
        // count residues, not atoms
        if ( i == 0 || QString::fromStdString( atoms[ i - 1 ].resName ) != rn ) {
            instance_count[ base ]++;
        }
    }

    std::set<QString> emitted;
    for ( size_t i = 0; i < atoms.size(); ++i ) {
        QString rn = QString::fromStdString( atoms[ i ].resName );
        if ( rn.isEmpty() || !to_perceive.count( rn ) ) {
            continue;
        }
        QString base = rn; base.remove( rx_nc );
        if ( emitted.count( base ) ) {
            continue;
        }
        emitted.insert( base );

        const std::string chain  = atoms[ i ].chain;
        const int         resSeq = atoms[ i ].resSeq;

        std::vector<InAtom>  res_in;
        std::vector<OutAtom> res_out;
        for ( size_t j = 0; j < atoms.size(); ++j ) {
            if ( QString::fromStdString( atoms[ j ].resName ) != rn ) continue;
            if ( atoms[ j ].chain != chain || atoms[ j ].resSeq != resSeq ) continue;
            res_in .push_back( atoms[ j ] );
            res_out.push_back( out  [ j ] );
        }
        if ( res_in.empty() ) {
            continue;
        }

        Tentative t;
        t.resName       = base;
        t.instances     = instance_count.count( base ) ? instance_count[ base ] : 1;
        t.chemical_name = hetnam.count( base ) ? hetnam[ base ] : QString();
        t.atoms         = (int) res_in.size();
        for ( size_t k = 0; k < res_out.size(); ++k ) {
            if ( res_out[ k ].ambiguity || !res_out[ k ].in_table ) {
                ++t.flagged;
            }
        }
        // Indices into the WHOLE structure: psv needs the surrounding bond graph to classify a
        // backbone amide N or a carboxyl O, and to see which rings lie inside this residue.
        std::vector<int> idx;
        for ( size_t j = 0; j < atoms.size(); ++j ) {
            if ( QString::fromStdString( atoms[ j ].resName ) != rn ) continue;
            if ( atoms[ j ].chain != chain || atoms[ j ].resSeq != resSeq ) continue;
            idx.push_back( (int) j );
        }
        somo_residue_builder::Built built =
            somo_residue_builder::build( base.toStdString(), atoms, out, bonds, idx, perc,
                                         hyd, t.chemical_name.toStdString(), opt );
        t.block = QString::fromStdString( built.residue_block );
        for ( size_t k = 0; k < built.new_hybrids.size(); ++k ) {
            t.new_hybrids << QString::fromStdString( built.new_hybrids[ k ] );
        }
        t.vbar   = built.psv.ok ? built.psv.vbar : 0.0;
        t.molvol = built.molvol;
        t.hydration = built.hydration.ok ? built.hydration.total : 0.0;
        result << t;
    }
    return result;
}

somo_hydration::Table hydration_from_residue_list( const std::vector<struct residue> & coded ) {
    std::vector< std::pair< std::string, double > > obs;
    // One vote per residue NAME. residue_list holds several entries for a residue that has
    // ionization variants, and counting each would weight those residues more heavily than the
    // rest purely because they are pH-aware -- which is enough to flip the majority for a
    // borderline type such as O2H1 (51 zeros against 67 ones).
    std::set<QString> counted;
    for ( size_t i = 0; i < coded.size(); ++i ) {
        if ( !counted.insert( coded[ i ].name ).second ) continue;
        for ( size_t a = 0; a < coded[ i ].r_atom.size(); ++a ) {
            const struct atom & at = coded[ i ].r_atom[ a ];
            if ( at.hybrid.name.isEmpty() ) continue;
            obs.push_back( std::make_pair( at.hybrid.name.toStdString(),
                                           (double) at.hydration ) );
        }
    }
    return somo_hydration::Table::from_observations( obs );
}

CompareResult compare_against_table(
    const PDB_model & model,
    const HybridTable & tbl,
    const std::map< QString, std::map< QString, QString > > & curated,
    const QString & pdb_filename ) {

    CompareResult r;
    std::vector<InAtom> atoms = from_pdb_model( model );
    if ( atoms.empty() ) {
        return r;
    }
    std::vector< std::pair< int, int > > ebonds;
    if ( !pdb_filename.isEmpty() ) {
        ebonds = conect_bonds( pdb_filename, atoms );
    }
    Perceiver perc( tbl );
    Bonds bonds;
    std::vector<OutAtom> out = perc.perceive( atoms, bonds, ebonds );

    QString last_key;
    for ( size_t i = 0; i < atoms.size(); ++i ) {
        QString rn = QString::fromStdString( atoms[ i ].resName );
        QString an = QString::fromStdString( atoms[ i ].name );
        auto ri = curated.find( rn );
        if ( ri == curated.end() ) {
            continue;                      // not a coded residue: nothing to compare against
        }
        auto ai = ri->second.find( an );
        if ( ai == ri->second.end() ) {
            continue;                      // atom not in the template (e.g. OXT, alt naming)
        }
        QString key = rn + "|" + QString::fromStdString( atoms[ i ].chain )
                      + "|" + QString::number( atoms[ i ].resSeq );
        if ( key != last_key ) { ++r.residues; last_key = key; }

        const QString & expected = ai->second;
        QString got = QString::fromStdString( out[ i ].hybrid );
        ++r.scored;
        bool e = ( got == expected );
        bool p = tbl.phys_equal( got.toStdString(), expected.toStdString() );
        if ( e ) ++r.exact;
        if ( p ) ++r.phys;
        if ( !p ) {
            CompareRow row;
            row.res = rn; row.atom = an; row.expected = expected; row.got = got;
            r.mismatches << row;
            r.by_pair[ expected + " -> " + got ]++;
        }
    }
    return r;
}

ValidateResult validate_against_table(
    const PDB_model & model,
    const HybridTable & tbl,
    const std::vector<struct residue> & coded,
    const somo_hydration::Table * hyd,
    const somo_residue_builder::Options & opt,
    const QString & pdb_filename ) {

    ValidateResult vr;
    std::vector<InAtom> atoms = from_pdb_model( model );
    if ( atoms.empty() ) {
        return vr;
    }

    // What somo.residue stores, keyed by residue name.
    struct Stored { double vbar = 0, molvol = 0, hydration = 0; int natoms = 0; bool ok = false; };
    std::map< QString, Stored > stored;
    for ( size_t i = 0; i < coded.size(); ++i ) {
        const struct residue & rr = coded[ i ];
        if ( stored.count( rr.name ) ) continue;              // first definition wins
        Stored st;
        st.vbar   = rr.vbar;
        st.molvol = rr.molvol;
        st.natoms = (int) rr.r_atom.size();
        for ( size_t a = 0; a < rr.r_atom.size(); ++a ) st.hydration += rr.r_atom[ a ].hydration;
        st.ok = true;
        stored[ rr.name ] = st;
    }

    Perceiver perc( tbl );
    std::vector< std::pair< int, int > > ebonds = conect_bonds( pdb_filename, atoms );
    Bonds bonds;
    std::vector<OutAtom> out = perc.perceive( atoms, bonds, ebonds );

    // group atoms by residue instance
    std::map< QString, std::vector<int> > inst;
    for ( size_t j = 0; j < atoms.size(); ++j ) {
        const QString rn = QString::fromStdString( atoms[ j ].resName );
        inst[ rn + "|" + QString::fromStdString( atoms[ j ].chain ) +
              QString::number( atoms[ j ].resSeq ) ].push_back( (int) j );
    }

    struct Acc { double vbar = 0, molvol = 0, hyd = 0; int n = 0; };
    std::map< QString, Acc > acc;

    for ( std::map< QString, std::vector<int> >::const_iterator it = inst.begin();
          it != inst.end(); ++it ) {
        const QString rn = it->first.left( it->first.indexOf( '|' ) );
        if ( !stored.count( rn ) || !stored[ rn ].ok ) continue;

        // A chain terminus carries an extra OXT, and a residue with unmodelled side-chain atoms
        // is not the molecule somo.residue describes. Either way the comparison would be against
        // different chemistry, so count and skip rather than quietly average them in.
        bool terminal = false;
        for ( size_t k = 0; k < it->second.size(); ++k ) {
            // C-terminus: the extra OXT. N-terminus: a free, protonated backbone nitrogen
            // instead of an amide -- different chemistry from the tabulated internal residue,
            // and worth several waters of hydration.
            if ( atoms[ it->second[ k ] ].name == "OXT" ) terminal = true;
            if ( atoms[ it->second[ k ] ].name == "N" &&
                 out[ it->second[ k ] ].formal_charge > 0 ) terminal = true;
        }
        if ( terminal ) { ++vr.skipped_terminal; continue; }
        if ( (int) it->second.size() != stored[ rn ].natoms ) { ++vr.skipped_incomplete; continue; }

        somo_residue_builder::Built b =
            somo_residue_builder::build( rn.toStdString(), atoms, out, bonds, it->second,
                                         perc, hyd, std::string(), opt );
        if ( !b.ok ) continue;
        Acc & a = acc[ rn ];
        a.vbar   += b.psv.ok ? b.psv.vbar : 0.0;
        a.molvol += b.molvol;
        a.hyd    += b.hydration.ok ? b.hydration.total : 0.0;
        ++a.n;
        ++vr.instances;
    }

    for ( std::map< QString, Acc >::const_iterator it = acc.begin(); it != acc.end(); ++it ) {
        if ( !it->second.n ) continue;
        ValidateRow row;
        row.resName            = it->first;
        row.instances          = it->second.n;
        row.vbar_computed      = it->second.vbar   / it->second.n;
        row.molvol_computed    = it->second.molvol / it->second.n;
        row.hydration_computed = it->second.hyd    / it->second.n;
        row.vbar_stored        = stored[ it->first ].vbar;
        row.molvol_stored      = stored[ it->first ].molvol;
        row.hydration_stored   = stored[ it->first ].hydration;
        vr.rows << row;
    }
    return vr;
}

} // namespace somo_perceive
