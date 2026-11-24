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
            // The element column is authoritative; fall back to the atom name only if absent.
            x.element = norm_element( ( pa.element.trimmed().isEmpty()
                                        ? pa.name : pa.element ).trimmed().toStdString() );
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
                                   const QString           & pdb_filename ) {
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
    std::vector<OutAtom> out = perc.perceive( atoms, ebonds );

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
        Perceiver::Emitted em =
            perc.emit_residue( base.toStdString(), res_in, res_out, t.chemical_name.toStdString() );
        t.block = QString::fromStdString( em.residue_block );
        for ( size_t k = 0; k < em.new_hybrids.size(); ++k ) {
            t.new_hybrids << QString::fromStdString( em.new_hybrids[ k ] );
        }
        result << t;
    }
    return result;
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
    std::vector<OutAtom> out = perc.perceive( atoms, ebonds );

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

} // namespace somo_perceive
