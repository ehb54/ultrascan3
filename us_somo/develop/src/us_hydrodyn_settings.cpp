// this is part of the class US_Hydrodyn
// listing of other files is in us_hydrodyn.cpp
// (this) us_hydrodyn_settings.cpp contains code for reading/writing settings

#include "../include/us_hydrodyn.h"
#define SLASH "/"

int US_Hydrodyn::read_config(QFile& f)
{
   QString str;
   // QTextStream( stdout ) << "read config\n";
   if ( f.fileName() == QString() ||
        f.fileName().isEmpty() )
   {
      return -1;
   }
   if ( f.handle() == -1 && !f.open(QIODevice::ReadOnly) )
   {
      return -2;
   }

   QTextStream ts(&f);

   // this is a really silly way to do this, carried over from legacy code
   // the config file should be free format
   // either XML, JSON or simply param value lines
   // this will provide easy updates and allow easy upgrading without removing previous parameters
   // i.e. via merging
   // similar things should also be done with the somo.residue, hybrid, atom & saxs_atoms files

   str = ts.readLine();

   if ( str == QString() ) return -10000; // first line is comment
   if ( str.contains( "JSON" ) )
   {
      QString qs;
      while ( !ts.atEnd() )
      {
         qs += ts.readLine() + "\n";
      }
      f.close();
      return load_config_json( qs ) ? 0 : -3;
   }

   hard_coded_defaults();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10001;
   replicate_o_r_method_somo = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10001;
   sidechain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10002;
   sidechain_overlap.fuse_beads = (bool) str.toInt();
   ts >> sidechain_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10003;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10004;
   sidechain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> sidechain_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10005;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10006;
   sidechain_overlap.remove_sync = (bool) str.toInt();
   ts >> sidechain_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10007;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10008;
   sidechain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10009;
   sidechain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10010;
   mainchain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10011;
   mainchain_overlap.fuse_beads = (bool) str.toInt();
   ts >> mainchain_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10012;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10013;
   mainchain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> mainchain_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10014;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10015;
   mainchain_overlap.remove_sync = (bool) str.toInt();
   ts >> mainchain_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10016;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10017;
   mainchain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10018;
   mainchain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10020;
   buried_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10021;
   buried_overlap.fuse_beads = (bool) str.toInt();
   ts >> buried_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10022;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10023;
   buried_overlap.remove_hierarch = (bool) str.toInt();
   ts >> buried_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10024;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10025;
   buried_overlap.remove_sync = (bool) str.toInt();
   ts >> buried_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10026;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10027;
   buried_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10028;
   buried_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10030;
   replicate_o_r_method_grid = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10030;
   grid_exposed_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10031;
   grid_exposed_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_exposed_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10032;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10033;
   grid_exposed_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_exposed_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10034;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10035;
   grid_exposed_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_exposed_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10036;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10037;
   grid_exposed_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10038;
   grid_exposed_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10040;
   grid_buried_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10041;
   grid_buried_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_buried_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10042;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10043;
   grid_buried_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_buried_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10044;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10045;
   grid_buried_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_buried_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10046;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10047;
   grid_buried_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10048;
   grid_buried_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10050;
   grid_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10051;
   grid_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString() ) return -10052;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10053;
   grid_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString() ) return -10054;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10055;
   grid_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_overlap.remove_sync_percent;
   if ( ts.readLine() == QString() ) return -10056;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10057;
   grid_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10058;
   grid_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10059;
   overlap_tolerance = str.toDouble();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10060;
   bead_output.output = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10061;
   bead_output.sequence = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10062;
   bead_output.correspondence = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10070;
   asa.probe_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10071;
   asa.probe_recheck_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10072;
   asa.threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10073;
   asa.threshold_percent = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10074;
   asa.grid_threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10075;
   asa.grid_threshold_percent = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10076;
   asa.calculation = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10077;
   asa.recheck_beads = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10078;
   asa.method = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10079;
   asa.asab1_step = str.toFloat();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10080;
   grid.cubic = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10081;
   grid.hydrate = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10082;
   grid.center = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10083;
   grid.tangency = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10084;
   grid.cube_side = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10085;
   grid.enable_asa = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10090;
   misc.hydrovol = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10091;
   misc.compute_vbar = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10092;
   misc.vbar = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10092;
   misc.vbar_temperature = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10093;
   misc.pb_rule_on = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10094;
   misc.avg_radius = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10095;
   misc.avg_mass = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10096;
   misc.avg_hydration = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10097;
   misc.avg_volume = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10098;
   misc.avg_vbar = str.toDouble();

   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.unit = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.solvent_name = str;
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.solvent_acronym = str.left(5);
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.temperature = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.solvent_viscosity = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.solvent_density = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10100;
   hydro.reference_system = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10101;
   hydro.boundary_cond = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10102;
   hydro.volume_correction = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10103;
   hydro.volume = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10104;
   hydro.mass_correction = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10105;
   hydro.mass = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10106;
   hydro.bead_inclusion = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10107;
   hydro.rotational = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10108;
   hydro.viscosity = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10109;
   hydro.overlap_cutoff = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -10110;
   hydro.overlap = str.toDouble();
   sidechain_overlap.title = "exposed side chain beads";
   mainchain_overlap.title = "exposed main/main and\nmain/side chain beads";
   buried_overlap.title = "buried beads";
   grid_exposed_overlap.title = "exposed grid beads";
   grid_buried_overlap.title = "buried grid beads";
   grid_overlap.title = "grid beads";

   // pdb_visualization options:

   ts >> str; // visualization option
   if ( ts.readLine() == QString() ) return -10120;
   pdb_vis.visualization = str.toInt();

   if ( ( str = ts.readLine() ) == QString() ) return -10121;
   pdb_vis.filename = str; // custom Rasmol script file

   // pdb_parsing options:

   ts >> str; // skip hydrogens?
   if ( ts.readLine() == QString() ) return -10130;
   pdb_parse.skip_hydrogen = (bool) str.toInt();
   ts >> str; // skip water molecules?
   if ( ts.readLine() == QString() ) return -10131;
   pdb_parse.skip_water = (bool) str.toInt();
   ts >> str; // skip alternate conformations?
   if ( ts.readLine() == QString() ) return -10132;
   pdb_parse.alternate = (bool) str.toInt();
   ts >> str; // find free SH atoms?
   if ( ts.readLine() == QString() ) return -10133;
   pdb_parse.find_sh = (bool) str.toInt();
   ts >> str; // missing residue choice
   if ( ts.readLine() == QString() ) return -10134;
   pdb_parse.missing_residues = str.toInt();
   ts >> str; // missing atom choice
   if ( ts.readLine() == QString() ) return -10135;
   pdb_parse.missing_atoms = str.toInt();

   ts >> str; // water electron density
   if ( ts.readLine() == QString() ) return -10140;
   saxs_options.water_e_density = str.toFloat();

   ts >> str; // H scattering length (*10^-12 cm)
   if ( ts.readLine() == QString() ) return -10141;
   saxs_options.h_scat_len = str.toFloat();
   ts >> str; // D scattering length (*10^-12 cm)
   if ( ts.readLine() == QString() ) return -10142;
   saxs_options.d_scat_len = str.toFloat();
   ts >> str; // H2O scattering length density (*10^-10 cm^2)
   if ( ts.readLine() == QString() ) return -10143;
   saxs_options.h2o_scat_len_dens = str.toFloat();
   ts >> str; // D2O scattering length density (*10^-10 cm^2)
   if ( ts.readLine() == QString() ) return -10144;
   saxs_options.d2o_scat_len_dens = str.toFloat();
   ts >> str; // D2O concentration (0 to 1)
   if ( ts.readLine() == QString() ) return -10145;
   saxs_options.d2o_conc = str.toFloat();
   ts >> str; // Fraction of exchanged peptide H (0 to 1)
   if ( ts.readLine() == QString() ) return -10146;
   saxs_options.frac_of_exch_pep = str.toFloat();

   ts >> str; // wavelength
   if ( ts.readLine() == QString() ) return -10148;
   saxs_options.wavelength = str.toFloat();
   ts >> str; // start angle
   if ( ts.readLine() == QString() ) return -10149;
   saxs_options.start_angle = str.toFloat();
   ts >> str; // end angle
   if ( ts.readLine() == QString() ) return -10150;
   saxs_options.end_angle = str.toFloat();
   ts >> str; // delta angle
   if ( ts.readLine() == QString() ) return -10151;
   saxs_options.delta_angle = str.toFloat();
   ts >> str; // maximum size
   if ( ts.readLine() == QString() ) return -10152;
   saxs_options.max_size = str.toFloat();
   ts >> str; // bin size
   if ( ts.readLine() == QString() ) return -10153;
   saxs_options.bin_size = str.toFloat();
   ts >> str; // hydrate pdb model?
   if ( ts.readLine() == QString() ) return -10154;
   saxs_options.hydrate_pdb = (bool) str.toInt();
   ts >> str; // curve
   if ( ts.readLine() == QString() ) return -10155;
   saxs_options.curve = str.toInt();
   ts >> str; // saxs_sans
   if ( ts.readLine() == QString() ) return -10156;
   saxs_options.saxs_sans = str.toInt();

   // should be saved
   // saxs_options.guinier_csv = false;
   // saxs_options.guinier_csv_filename = "guinier";
   // saxs_options.qRgmax = 1.3e0;
   // saxs_options.qstart = 1e-7;
   // saxs_options.qend = .5e0;
   // saxs_options.pointsmin = 10;
   // saxs_options.pointsmax = 100;

   // bd_options
   {
      int i = -12000;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.threshold_pb_pb = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.threshold_pb_sc = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.threshold_sc_sc = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.do_rr = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.force_chem = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.bead_size_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.show_pdb = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.run_browflex = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.tprev = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.ttraj = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.deltat = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.npadif = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.nconf = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.inter = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.iorder = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.iseed = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.icdm = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_pb_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_pb_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_pb_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_pb_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_pb_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_pb_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_pb_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_pb_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_pb_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_sc_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_sc_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_sc_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_sc_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_sc_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_chem_sc_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.chem_sc_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_pb_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_pb_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_pb_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_pb_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_pb_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_pb_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_pb_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_pb_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.pb_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.sc_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_sc_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.sc_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_sc_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.sc_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.compute_sc_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.sc_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      bd_options.nmol = str.toInt();
      i--;
   }

   // anaflex_options
   {
      int i = -13000;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_anaflex = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.nfrec = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.instprofiles = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_12 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_13 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_18 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_20 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_1_24 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_12 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_13 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_18 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_20 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_2_24 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_9 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_10 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_15 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_16 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_4_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_4_6 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_4_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_4_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_9 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.ntimc = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.tmax = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_5_iii = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_5_jjj = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_10_theta = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_10_refractive_index = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_10_lambda = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_14_iii = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString() ) return i;
      anaflex_options.run_mode_3_14_jjj = str.toInt();
      i--;
   }

   ts >> str; // batch missing atom handling
   if ( ts.readLine() == QString() ) return -11000;
   batch.missing_atoms = str.toInt();
   ts >> str; // batch missing residue handling
   if ( ts.readLine() == QString() ) return -11001;
   batch.missing_residues = str.toInt();
   ts >> str; // batch run somo
   if ( ts.readLine() == QString() ) return -11002;
   batch.somo = (bool) str.toInt();
   ts >> str; // batch run grid
   if ( ts.readLine() == QString() ) return -11003;
   batch.grid = (bool) str.toInt();
   ts >> str; // batch run hydro
   if ( ts.readLine() == QString() ) return -11004;
   batch.hydro = (bool) str.toInt();
   ts >> str; // batch avg hydro
   if ( ts.readLine() == QString() ) return -11005;
   batch.avg_hydro = (bool) str.toInt();
   ts >> str; // batch avg hydro name
   if ( ts.readLine() == QString() ) return -11006;
   batch.avg_hydro_name = str;
   ts >> str; // batch height
   if ( ts.readLine() == QString() ) return -11007;
   batch.height = str.toInt();
   ts >> str; // batch width
   if ( ts.readLine() == QString() ) return -11008;
   batch.width = str.toInt();

   batch.file.clear( );
   ts >> str; // batch file list
   if ( ts.readLine() == QString() ) return -11010;
   {
      int number_of_files = str.toInt();
      for ( int i = 0; i < number_of_files; i++ )
      {
         if ( (str = ts.readLine() ) == QString() ) return -(11100 + i);
         batch.file.push_back(str);
      }
   }

   save_params.field.clear( );
   ts >> str; // save field list
   if ( ts.readLine() == QString() ) return -11500;
   {
      int number_of_fields = str.toInt();
      for ( int i = 0; i < number_of_fields; i++ )
      {
         if ( (str = ts.readLine() ) == QString() ) return -(11501 + i);
         save_params.field.push_back(str);
      }
   }

   if ( ( str = ts.readLine() ) == QString() ) return -11600;
   path_load_pdb = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11601;
   path_view_pdb = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11602;
   path_load_bead_model = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11603;
   path_view_asa_res = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11604;
   path_view_bead_model = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11605;
   path_open_hydro_res = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11606;
   saxs_options.path_load_saxs_curve = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11607;
   saxs_options.path_load_gnom = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11608;
   saxs_options.path_load_prr = str;

   ts >> str;
   if ( ts.readLine()  == QString() ) return -11609;
   asa.hydrate_probe_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine()  == QString() ) return -11610;
   asa.hydrate_threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11611;
   misc.target_e_density       = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11612;
   misc.target_volume          = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11613;
   misc.set_target_on_load_pdb = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11614;
   misc.equalize_radii         = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11615;
   dmd_options.force_chem = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11616;
   dmd_options.pdb_static_pairs = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11617;
   dmd_options.threshold_pb_pb = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11618;
   dmd_options.threshold_pb_sc = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11619;
   dmd_options.threshold_sc_sc = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11620;
   saxs_options.normalize_by_mw = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11621;
   saxs_options.saxs_iq_native_debye = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11622;
   saxs_options.saxs_iq_native_hybrid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11623;
   saxs_options.saxs_iq_native_hybrid2 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11624;
   saxs_options.saxs_iq_native_hybrid3 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11625;
   saxs_options.saxs_iq_native_fast = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11626;
   saxs_options.saxs_iq_native_fast_compute_pr = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11627;
   saxs_options.saxs_iq_foxs = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11628;
   saxs_options.saxs_iq_crysol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11629;
   saxs_options.sans_iq_native_debye = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11630;
   saxs_options.sans_iq_native_hybrid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11631;
   saxs_options.sans_iq_native_hybrid2 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11632;
   saxs_options.sans_iq_native_hybrid3 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11633;
   saxs_options.sans_iq_native_fast = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11634;
   saxs_options.sans_iq_native_fast_compute_pr = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11635;
   saxs_options.sans_iq_cryson = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11636;
   saxs_options.hybrid2_q_points = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11637;
   saxs_options.iq_ask = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11638;
   saxs_options.iq_scale_ask = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11639;
   saxs_options.iq_scale_angstrom = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11640;
   saxs_options.iq_scale_nm = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11641;
   saxs_options.sh_max_harmonics = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11642;
   saxs_options.sh_fibonacci_grid_order = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11643;
   saxs_options.crysol_hydration_shell_contrast = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11644;
   saxs_options.crysol_default_load_difference_intensity = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11645;
   saxs_options.crysol_version_26 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11646;
   saxs_options.fast_bin_size = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11647;
   saxs_options.fast_modulation = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11648;
   saxs_options.compute_saxs_coeff_for_bead_models = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11649;
   saxs_options.compute_sans_coeff_for_bead_models = (bool)str.toInt();

   if ( ( str = ts.readLine() ) == QString() ) return -11650;
   saxs_options.default_atom_filename = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11651;
   saxs_options.default_hybrid_filename = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11652;
   saxs_options.default_saxs_filename = str;
   if ( ( str = ts.readLine() ) == QString() ) return -11653;
   saxs_options.default_rotamer_filename = str;

   ts >> str;
   if ( ts.readLine() == QString() ) return -11654;
   saxs_options.steric_clash_distance         = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11655;
   saxs_options.steric_clash_recheck_distance = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11656;
   saxs_options.disable_iq_scaling = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11657;
   saxs_options.autocorrelate = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11658;
   saxs_options.hybrid_radius_excl_vol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11659;
   saxs_options.scale_excl_vol = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11660;
   saxs_options.subtract_radius = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11661;
   saxs_options.iqq_scale_minq = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11662;
   saxs_options.iqq_scale_maxq = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11663;
   saxs_options.iqq_scale_nnls = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11664;
   saxs_options.iqq_scale_linear_offset = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11665;
   saxs_options.iqq_scale_chi2_fitting = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11666;
   saxs_options.iqq_expt_data_contains_variances = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11667;
   saxs_options.iqq_ask_target_grid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11668;
   saxs_options.iqq_scale_play = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11669;
   saxs_options.swh_excl_vol = str.toFloat();
   if ( ( str = ts.readLine() ) == QString() ) return -11670;
   saxs_options.iqq_default_scaling_target = str;
   ts >> str;
   if ( ts.readLine() == QString() ) return -11671;
   saxs_options.saxs_iq_hybrid_adaptive = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11672;
   saxs_options.sans_iq_hybrid_adaptive = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11673;
   saxs_options.bead_model_rayleigh   = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11674;
   saxs_options.iqq_log_fitting       = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11675;
   saxs_options.iqq_kratky_fit        = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11676;
   saxs_options.iqq_use_atomic_ff     = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11677;
   saxs_options.iqq_use_saxs_excl_vol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11678;
   saxs_options.alt_hydration         = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11679;
   saxs_options.xsr_symmop                = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11680;
   saxs_options.xsr_nx                    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11681;
   saxs_options.xsr_ny                    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11682;
   saxs_options.xsr_griddistance          = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11683;
   saxs_options.xsr_ncomponents           = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11684;
   saxs_options.xsr_compactness_weight    = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11685;
   saxs_options.xsr_looseness_weight      = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11686;
   saxs_options.xsr_temperature           = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11687;
   hydro.zeno_zeno              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11688;
   hydro.zeno_interior          = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11689;
   hydro.zeno_surface           = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11690;
   hydro.zeno_zeno_steps        = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11691;
   hydro.zeno_interior_steps    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11692;
   hydro.zeno_surface_steps     = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11693;
   hydro.zeno_surface_thickness = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11694;
   misc.hydro_supc              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11695;
   misc.hydro_zeno              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11696;
   batch.saxs_search = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString() ) return -11697;
   batch.zeno        = (bool)str.toInt();

   if ( !ts.atEnd() ) return -20000;

   f.close();
   return 0;
}

int US_Hydrodyn::read_config(const QString& fname)
{
   QFile f;
   QString str;
   if (fname.isEmpty()) // this is true if we don't explicitely set a user-defined file name
   {
      f.setFileName( US_Config::get_home_dir() + "etc/somo.config");
   }
   else
   {
      f.setFileName(fname);
   }
   if (!f.open(QIODevice::ReadOnly)) // first try user's directory for default settings
   {
      if ( !guiFlag )
      {
         return -1;
      }
      f.setFileName( US_Config::get_home_dir() + "etc/somo.config");
      if (!f.open(QIODevice::ReadOnly)) // read system directory
      {
         reset(); // no file available, reset and return
         return 0;
      }
   }
   return read_config(f);
}

void US_Hydrodyn::write_config(const QString& fname)
{
   if ( misc.restore_pb_rule ) {
      // us_qdebug( "write_config() restoring pb rule" );
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
         
      misc.pb_rule_on      = true;
      misc.restore_pb_rule = false;
   }

   QFile f;
   QString str;
   f.setFileName( fname );
   QTextStream( stdout ) << fname << Qt::endl;
   if ( f.open( QIODevice::WriteOnly ) ) // first try user's directory for default settings
   {
      QTextStream ts(&f);
      
      map < QString, QString > parameters;

      ts << "US-SOMO JSON Config file\n";
      
      parameters[ "replicate_o_r_method_somo" ] = QString( "%1" ).arg( replicate_o_r_method_somo );
      parameters[ "sidechain_overlap.remove_overlap" ] = QString( "%1" ).arg( sidechain_overlap.remove_overlap );
      parameters[ "sidechain_overlap.fuse_beads" ] = QString( "%1" ).arg( sidechain_overlap.fuse_beads );
      parameters[ "sidechain_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( sidechain_overlap.fuse_beads_percent );
      parameters[ "sidechain_overlap.remove_hierarch" ] = QString( "%1" ).arg( sidechain_overlap.remove_hierarch );
      parameters[ "sidechain_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( sidechain_overlap.remove_hierarch_percent );
      parameters[ "sidechain_overlap.remove_sync" ] = QString( "%1" ).arg( sidechain_overlap.remove_sync );
      parameters[ "sidechain_overlap.remove_sync_percent" ] = QString( "%1" ).arg( sidechain_overlap.remove_sync_percent );
      parameters[ "sidechain_overlap.translate_out" ] = QString( "%1" ).arg( sidechain_overlap.translate_out );
      parameters[ "sidechain_overlap.show_translate" ] = QString( "%1" ).arg( sidechain_overlap.show_translate );
      parameters[ "mainchain_overlap.remove_overlap" ] = QString( "%1" ).arg( mainchain_overlap.remove_overlap );
      parameters[ "mainchain_overlap.fuse_beads" ] = QString( "%1" ).arg( mainchain_overlap.fuse_beads );
      parameters[ "mainchain_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( mainchain_overlap.fuse_beads_percent );
      parameters[ "mainchain_overlap.remove_hierarch" ] = QString( "%1" ).arg( mainchain_overlap.remove_hierarch );
      parameters[ "mainchain_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( mainchain_overlap.remove_hierarch_percent );
      parameters[ "mainchain_overlap.remove_sync" ] = QString( "%1" ).arg( mainchain_overlap.remove_sync );
      parameters[ "mainchain_overlap.remove_sync_percent" ] = QString( "%1" ).arg( mainchain_overlap.remove_sync_percent );
      parameters[ "mainchain_overlap.translate_out" ] = QString( "%1" ).arg( mainchain_overlap.translate_out );
      parameters[ "mainchain_overlap.show_translate" ] = QString( "%1" ).arg( mainchain_overlap.show_translate );
      parameters[ "buried_overlap.remove_overlap" ] = QString( "%1" ).arg( buried_overlap.remove_overlap );
      parameters[ "buried_overlap.fuse_beads" ] = QString( "%1" ).arg( buried_overlap.fuse_beads );
      parameters[ "buried_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( buried_overlap.fuse_beads_percent );
      parameters[ "buried_overlap.remove_hierarch" ] = QString( "%1" ).arg( buried_overlap.remove_hierarch );
      parameters[ "buried_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( buried_overlap.remove_hierarch_percent );
      parameters[ "buried_overlap.remove_sync" ] = QString( "%1" ).arg( buried_overlap.remove_sync );
      parameters[ "buried_overlap.remove_sync_percent" ] = QString( "%1" ).arg( buried_overlap.remove_sync_percent );
      parameters[ "buried_overlap.translate_out" ] = QString( "%1" ).arg( buried_overlap.translate_out );
      parameters[ "buried_overlap.show_translate" ] = QString( "%1" ).arg( buried_overlap.show_translate );
      parameters[ "replicate_o_r_method_grid" ] = QString( "%1" ).arg( replicate_o_r_method_grid );
      parameters[ "grid_exposed_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_overlap );
      parameters[ "grid_exposed_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_exposed_overlap.fuse_beads );
      parameters[ "grid_exposed_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.fuse_beads_percent );
      parameters[ "grid_exposed_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_hierarch );
      parameters[ "grid_exposed_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_hierarch_percent );
      parameters[ "grid_exposed_overlap.remove_sync" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_sync );
      parameters[ "grid_exposed_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_sync_percent );
      parameters[ "grid_exposed_overlap.translate_out" ] = QString( "%1" ).arg( grid_exposed_overlap.translate_out );
      parameters[ "grid_exposed_overlap.show_translate" ] = QString( "%1" ).arg( grid_exposed_overlap.show_translate );
      parameters[ "grid_buried_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_buried_overlap.remove_overlap );
      parameters[ "grid_buried_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_buried_overlap.fuse_beads );
      parameters[ "grid_buried_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_buried_overlap.fuse_beads_percent );
      parameters[ "grid_buried_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_buried_overlap.remove_hierarch );
      parameters[ "grid_buried_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_buried_overlap.remove_hierarch_percent );
      parameters[ "grid_buried_overlap.remove_sync" ] = QString( "%1" ).arg( grid_buried_overlap.remove_sync );
      parameters[ "grid_buried_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_buried_overlap.remove_sync_percent );
      parameters[ "grid_buried_overlap.translate_out" ] = QString( "%1" ).arg( grid_buried_overlap.translate_out );
      parameters[ "grid_buried_overlap.show_translate" ] = QString( "%1" ).arg( grid_buried_overlap.show_translate );
      parameters[ "grid_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_overlap.remove_overlap );
      parameters[ "grid_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_overlap.fuse_beads );
      parameters[ "grid_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_overlap.fuse_beads_percent );
      parameters[ "grid_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_overlap.remove_hierarch );
      parameters[ "grid_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_overlap.remove_hierarch_percent );
      parameters[ "grid_overlap.remove_sync" ] = QString( "%1" ).arg( grid_overlap.remove_sync );
      parameters[ "grid_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_overlap.remove_sync_percent );
      parameters[ "grid_overlap.translate_out" ] = QString( "%1" ).arg( grid_overlap.translate_out );
      parameters[ "grid_overlap.show_translate" ] = QString( "%1" ).arg( grid_overlap.show_translate );
      parameters[ "overlap_tolerance" ] = QString( "%1" ).arg( overlap_tolerance );
      parameters[ "bead_output.output" ] = QString( "%1" ).arg( bead_output.output );
      parameters[ "bead_output.sequence" ] = QString( "%1" ).arg( bead_output.sequence );
      parameters[ "bead_output.correspondence" ] = QString( "%1" ).arg( bead_output.correspondence );
      parameters[ "asa.probe_radius" ] = QString( "%1" ).arg( asa.probe_radius );
      parameters[ "asa.probe_recheck_radius" ] = QString( "%1" ).arg( asa.probe_recheck_radius );
      parameters[ "asa.threshold" ] = QString( "%1" ).arg( asa.threshold );
      parameters[ "asa.threshold_percent" ] = QString( "%1" ).arg( asa.threshold_percent );
      parameters[ "asa.vdw_grpy_probe_radius" ] = QString( "%1" ).arg( asa.vdw_grpy_probe_radius );
      parameters[ "asa.vdw_grpy_threshold_percent" ] = QString( "%1" ).arg( asa.vdw_grpy_threshold_percent );
      parameters[ "asa.grid_threshold" ] = QString( "%1" ).arg( asa.grid_threshold );
      parameters[ "asa.grid_threshold_percent" ] = QString( "%1" ).arg( asa.grid_threshold_percent );
      parameters[ "asa.calculation" ] = QString( "%1" ).arg( asa.calculation );
      parameters[ "asa.recheck_beads" ] = QString( "%1" ).arg( asa.recheck_beads );
      parameters[ "asa.method" ] = QString( "%1" ).arg( asa.method );
      parameters[ "asa.asab1_step" ] = QString( "%1" ).arg( asa.asab1_step );
      parameters[ "grid.cubic" ] = QString( "%1" ).arg( grid.cubic );
      parameters[ "grid.hydrate" ] = QString( "%1" ).arg( grid.hydrate );
      parameters[ "grid.center" ] = QString( "%1" ).arg( grid.center );
      parameters[ "grid.tangency" ] = QString( "%1" ).arg( grid.tangency );
      parameters[ "grid.cube_side" ] = QString( "%1" ).arg( grid.cube_side );
      parameters[ "grid.enable_asa" ] = QString( "%1" ).arg( grid.enable_asa );
      parameters[ "misc.hydrovol" ] = QString( "%1" ).arg( misc.hydrovol );
      parameters[ "misc.compute_vbar" ] = QString( "%1" ).arg( misc.compute_vbar );
      parameters[ "misc.vbar" ] = QString( "%1" ).arg( misc.vbar );
      parameters[ "misc.vbar_temperature" ] = QString( "%1" ).arg( misc.vbar_temperature );
      parameters[ "misc.pb_rule_on" ] = QString( "%1" ).arg( misc.pb_rule_on );
      parameters[ "misc.avg_radius" ] = QString( "%1" ).arg( misc.avg_radius );
      parameters[ "misc.avg_mass" ] = QString( "%1" ).arg( misc.avg_mass );
      parameters[ "misc.avg_num_elect" ] = QString( "%1" ).arg( misc.avg_num_elect );
      parameters[ "misc.avg_protons" ] = QString( "%1" ).arg( misc.avg_protons );
      parameters[ "misc.avg_hydration" ] = QString( "%1" ).arg( misc.avg_hydration );
      parameters[ "misc.avg_volume" ] = QString( "%1" ).arg( misc.avg_volume );
      parameters[ "misc.avg_vbar" ] = QString( "%1" ).arg( misc.avg_vbar );
      parameters[ "hydro.unit" ] = QString( "%1" ).arg( hydro.unit );
      parameters[ "hydro.solvent_name" ] = QString( "%1" ).arg( hydro.solvent_name );
      parameters[ "hydro.solvent_acronym" ] = QString( "%1" ).arg( hydro.solvent_acronym );
      parameters[ "hydro.temperature" ] = QString( "%1" ).arg( hydro.temperature );
      parameters[ "hydro.pH" ] = QString( "%1" ).arg( hydro.pH );
      parameters[ "hydro.solvent_viscosity" ] = QString( "%1" ).arg( hydro.solvent_viscosity );
      parameters[ "hydro.solvent_density" ] = QString( "%1" ).arg( hydro.solvent_density );
      parameters[ "hydro.reference_system" ] = QString( "%1" ).arg( hydro.reference_system );
      parameters[ "hydro.boundary_cond" ] = QString( "%1" ).arg( hydro.boundary_cond );
      parameters[ "hydro.volume_correction" ] = QString( "%1" ).arg( hydro.volume_correction );
      parameters[ "hydro.use_avg_for_volume" ] = QString( "%1" ).arg( hydro.use_avg_for_volume );
      parameters[ "hydro.volume" ] = QString( "%1" ).arg( hydro.volume );
      parameters[ "hydro.mass_correction" ] = QString( "%1" ).arg( hydro.mass_correction );
      parameters[ "hydro.mass" ] = QString( "%1" ).arg( hydro.mass );
      parameters[ "hydro.bead_inclusion" ] = QString( "%1" ).arg( hydro.bead_inclusion );
      parameters[ "hydro.grpy_bead_inclusion" ] = QString( "%1" ).arg( hydro.grpy_bead_inclusion );
      parameters[ "hydro.rotational" ] = QString( "%1" ).arg( hydro.rotational );
      parameters[ "hydro.viscosity" ] = QString( "%1" ).arg( hydro.viscosity );
      parameters[ "hydro.overlap_cutoff" ] = QString( "%1" ).arg( hydro.overlap_cutoff );
      parameters[ "hydro.overlap" ] = QString( "%1" ).arg( hydro.overlap );
      parameters[ "pdb_vis.visualization" ] = QString( "%1" ).arg( pdb_vis.visualization );
      parameters[ "pdb_vis.filename" ] = QString( "%1" ).arg( pdb_vis.filename );
      parameters[ "pdb_parse.skip_hydrogen" ] = QString( "%1" ).arg( pdb_parse.skip_hydrogen );
      parameters[ "pdb_parse.skip_water" ] = QString( "%1" ).arg( pdb_parse.skip_water );
      parameters[ "pdb_parse.alternate" ] = QString( "%1" ).arg( pdb_parse.alternate );
      parameters[ "pdb_parse.find_sh" ] = QString( "%1" ).arg( pdb_parse.find_sh );
      parameters[ "pdb_parse.missing_residues" ] = QString( "%1" ).arg( pdb_parse.missing_residues );
      parameters[ "pdb_parse.missing_atoms" ] = QString( "%1" ).arg( pdb_parse.missing_atoms );
      parameters[ "saxs_options.water_e_density" ] = QString( "%1" ).arg( saxs_options.water_e_density );
      parameters[ "saxs_options.h_scat_len" ] = QString( "%1" ).arg( saxs_options.h_scat_len );
      parameters[ "saxs_options.d_scat_len" ] = QString( "%1" ).arg( saxs_options.d_scat_len );
      parameters[ "saxs_options.h2o_scat_len_dens" ] = QString( "%1" ).arg( saxs_options.h2o_scat_len_dens );
      parameters[ "saxs_options.d2o_scat_len_dens" ] = QString( "%1" ).arg( saxs_options.d2o_scat_len_dens );
      parameters[ "saxs_options.d2o_conc" ] = QString( "%1" ).arg( saxs_options.d2o_conc );
      parameters[ "saxs_options.frac_of_exch_pep" ] = QString( "%1" ).arg( saxs_options.frac_of_exch_pep );
      parameters[ "saxs_options.wavelength" ] = QString( "%1" ).arg( saxs_options.wavelength );
      parameters[ "saxs_options.start_angle" ] = QString( "%1" ).arg( saxs_options.start_angle );
      parameters[ "saxs_options.end_angle" ] = QString( "%1" ).arg( saxs_options.end_angle );
      parameters[ "saxs_options.delta_angle" ] = QString( "%1" ).arg( saxs_options.delta_angle );
      parameters[ "saxs_options.max_size" ] = QString( "%1" ).arg( saxs_options.max_size );
      parameters[ "saxs_options.bin_size" ] = QString( "%1" ).arg( saxs_options.bin_size );
      parameters[ "saxs_options.hydrate_pdb" ] = QString( "%1" ).arg( saxs_options.hydrate_pdb );
      parameters[ "saxs_options.curve" ] = QString( "%1" ).arg( saxs_options.curve );
      parameters[ "saxs_options.saxs_sans" ] = QString( "%1" ).arg( saxs_options.saxs_sans );
      parameters[ "bd_options.threshold_pb_pb" ] = QString( "%1" ).arg( bd_options.threshold_pb_pb );
      parameters[ "bd_options.threshold_pb_sc" ] = QString( "%1" ).arg( bd_options.threshold_pb_sc );
      parameters[ "bd_options.threshold_sc_sc" ] = QString( "%1" ).arg( bd_options.threshold_sc_sc );
      parameters[ "bd_options.do_rr" ] = QString( "%1" ).arg( bd_options.do_rr );
      parameters[ "bd_options.force_chem" ] = QString( "%1" ).arg( bd_options.force_chem );
      parameters[ "bd_options.bead_size_type" ] = QString( "%1" ).arg( bd_options.bead_size_type );
      parameters[ "bd_options.show_pdb" ] = QString( "%1" ).arg( bd_options.show_pdb );
      parameters[ "bd_options.run_browflex" ] = QString( "%1" ).arg( bd_options.run_browflex );
      parameters[ "bd_options.tprev" ] = QString( "%1" ).arg( bd_options.tprev );
      parameters[ "bd_options.ttraj" ] = QString( "%1" ).arg( bd_options.ttraj );
      parameters[ "bd_options.deltat" ] = QString( "%1" ).arg( bd_options.deltat );
      parameters[ "bd_options.npadif" ] = QString( "%1" ).arg( bd_options.npadif );
      parameters[ "bd_options.nconf" ] = QString( "%1" ).arg( bd_options.nconf );
      parameters[ "bd_options.inter" ] = QString( "%1" ).arg( bd_options.inter );
      parameters[ "bd_options.iorder" ] = QString( "%1" ).arg( bd_options.iorder );
      parameters[ "bd_options.iseed" ] = QString( "%1" ).arg( bd_options.iseed );
      parameters[ "bd_options.icdm" ] = QString( "%1" ).arg( bd_options.icdm );
      parameters[ "bd_options.chem_pb_pb_bond_type" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_bond_type );
      parameters[ "bd_options.compute_chem_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_force_constant );
      parameters[ "bd_options.chem_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_force_constant );
      parameters[ "bd_options.compute_chem_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_equilibrium_dist );
      parameters[ "bd_options.chem_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_equilibrium_dist );
      parameters[ "bd_options.compute_chem_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_max_elong );
      parameters[ "bd_options.chem_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_max_elong );
      parameters[ "bd_options.chem_pb_sc_bond_type" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_bond_type );
      parameters[ "bd_options.compute_chem_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_force_constant );
      parameters[ "bd_options.chem_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_force_constant );
      parameters[ "bd_options.compute_chem_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_equilibrium_dist );
      parameters[ "bd_options.chem_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_equilibrium_dist );
      parameters[ "bd_options.compute_chem_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_max_elong );
      parameters[ "bd_options.chem_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_max_elong );
      parameters[ "bd_options.chem_sc_sc_bond_type" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_bond_type );
      parameters[ "bd_options.compute_chem_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_force_constant );
      parameters[ "bd_options.chem_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_force_constant );
      parameters[ "bd_options.compute_chem_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_equilibrium_dist );
      parameters[ "bd_options.chem_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_equilibrium_dist );
      parameters[ "bd_options.compute_chem_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_max_elong );
      parameters[ "bd_options.chem_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_max_elong );
      parameters[ "bd_options.pb_pb_bond_type" ] = QString( "%1" ).arg( bd_options.pb_pb_bond_type );
      parameters[ "bd_options.compute_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_force_constant );
      parameters[ "bd_options.pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.pb_pb_force_constant );
      parameters[ "bd_options.compute_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_equilibrium_dist );
      parameters[ "bd_options.pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.pb_pb_equilibrium_dist );
      parameters[ "bd_options.compute_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_max_elong );
      parameters[ "bd_options.pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.pb_pb_max_elong );
      parameters[ "bd_options.pb_sc_bond_type" ] = QString( "%1" ).arg( bd_options.pb_sc_bond_type );
      parameters[ "bd_options.compute_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_force_constant );
      parameters[ "bd_options.pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.pb_sc_force_constant );
      parameters[ "bd_options.compute_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_equilibrium_dist );
      parameters[ "bd_options.pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.pb_sc_equilibrium_dist );
      parameters[ "bd_options.compute_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_max_elong );
      parameters[ "bd_options.pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.pb_sc_max_elong );
      parameters[ "bd_options.sc_sc_bond_type" ] = QString( "%1" ).arg( bd_options.sc_sc_bond_type );
      parameters[ "bd_options.compute_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_force_constant );
      parameters[ "bd_options.sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.sc_sc_force_constant );
      parameters[ "bd_options.compute_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_equilibrium_dist );
      parameters[ "bd_options.sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.sc_sc_equilibrium_dist );
      parameters[ "bd_options.compute_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_max_elong );
      parameters[ "bd_options.sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.sc_sc_max_elong );
      parameters[ "bd_options.nmol" ] = QString( "%1" ).arg( bd_options.nmol );
      parameters[ "anaflex_options.run_anaflex" ] = QString( "%1" ).arg( anaflex_options.run_anaflex );
      parameters[ "anaflex_options.nfrec" ] = QString( "%1" ).arg( anaflex_options.nfrec );
      parameters[ "anaflex_options.instprofiles" ] = QString( "%1" ).arg( anaflex_options.instprofiles );
      parameters[ "anaflex_options.run_mode_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_1 );
      parameters[ "anaflex_options.run_mode_1_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_1 );
      parameters[ "anaflex_options.run_mode_1_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_2 );
      parameters[ "anaflex_options.run_mode_1_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_3 );
      parameters[ "anaflex_options.run_mode_1_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_4 );
      parameters[ "anaflex_options.run_mode_1_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_5 );
      parameters[ "anaflex_options.run_mode_1_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_7 );
      parameters[ "anaflex_options.run_mode_1_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_8 );
      parameters[ "anaflex_options.run_mode_1_12" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_12 );
      parameters[ "anaflex_options.run_mode_1_13" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_13 );
      parameters[ "anaflex_options.run_mode_1_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_14 );
      parameters[ "anaflex_options.run_mode_1_18" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_18 );
      parameters[ "anaflex_options.run_mode_1_20" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_20 );
      parameters[ "anaflex_options.run_mode_1_24" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_24 );
      parameters[ "anaflex_options.run_mode_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_2 );
      parameters[ "anaflex_options.run_mode_2_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_1 );
      parameters[ "anaflex_options.run_mode_2_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_2 );
      parameters[ "anaflex_options.run_mode_2_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_3 );
      parameters[ "anaflex_options.run_mode_2_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_4 );
      parameters[ "anaflex_options.run_mode_2_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_5 );
      parameters[ "anaflex_options.run_mode_2_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_7 );
      parameters[ "anaflex_options.run_mode_2_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_8 );
      parameters[ "anaflex_options.run_mode_2_12" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_12 );
      parameters[ "anaflex_options.run_mode_2_13" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_13 );
      parameters[ "anaflex_options.run_mode_2_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_14 );
      parameters[ "anaflex_options.run_mode_2_18" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_18 );
      parameters[ "anaflex_options.run_mode_2_20" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_20 );
      parameters[ "anaflex_options.run_mode_2_24" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_24 );
      parameters[ "anaflex_options.run_mode_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_3 );
      parameters[ "anaflex_options.run_mode_3_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_1 );
      parameters[ "anaflex_options.run_mode_3_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5 );
      parameters[ "anaflex_options.run_mode_3_9" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_9 );
      parameters[ "anaflex_options.run_mode_3_10" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10 );
      parameters[ "anaflex_options.run_mode_3_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14 );
      parameters[ "anaflex_options.run_mode_3_15" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_15 );
      parameters[ "anaflex_options.run_mode_3_16" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_16 );
      parameters[ "anaflex_options.run_mode_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_4 );
      parameters[ "anaflex_options.run_mode_4_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_1 );
      parameters[ "anaflex_options.run_mode_4_6" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_6 );
      parameters[ "anaflex_options.run_mode_4_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_7 );
      parameters[ "anaflex_options.run_mode_4_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_8 );
      parameters[ "anaflex_options.run_mode_9" ] = QString( "%1" ).arg( anaflex_options.run_mode_9 );
      parameters[ "anaflex_options.ntimc" ] = QString( "%1" ).arg( anaflex_options.ntimc );
      parameters[ "anaflex_options.tmax" ] = QString( "%1" ).arg( anaflex_options.tmax );
      parameters[ "anaflex_options.run_mode_3_5_iii" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5_iii );
      parameters[ "anaflex_options.run_mode_3_5_jjj" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5_jjj );
      parameters[ "anaflex_options.run_mode_3_10_theta" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_theta );
      parameters[ "anaflex_options.run_mode_3_10_refractive_index" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_refractive_index );
      parameters[ "anaflex_options.run_mode_3_10_lambda" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_lambda );
      parameters[ "anaflex_options.run_mode_3_14_iii" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14_iii );
      parameters[ "anaflex_options.run_mode_3_14_jjj" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14_jjj );
      parameters[ "batch.missing_atoms" ] = QString( "%1" ).arg( batch.missing_atoms );
      parameters[ "batch.missing_residues" ] = QString( "%1" ).arg( batch.missing_residues );
      parameters[ "batch.somo" ] = QString( "%1" ).arg( batch.somo );
      parameters[ "batch.grid" ] = QString( "%1" ).arg( batch.grid );
      parameters[ "batch.hydro" ] = QString( "%1" ).arg( batch.hydro );
      parameters[ "batch.avg_hydro" ] = QString( "%1" ).arg( batch.avg_hydro );
      parameters[ "batch.avg_hydro_name" ] = QString( "%1" ).arg( batch.avg_hydro_name );
      parameters[ "batch.height" ] = QString( "%1" ).arg( batch.height );
      parameters[ "batch.width" ] = QString( "%1" ).arg( batch.width );
      parameters[ "path_load_pdb" ] = QString( "%1" ).arg( path_load_pdb );
      parameters[ "path_view_pdb" ] = QString( "%1" ).arg( path_view_pdb );
      parameters[ "path_load_bead_model" ] = QString( "%1" ).arg( path_load_bead_model );
      parameters[ "path_view_asa_res" ] = QString( "%1" ).arg( path_view_asa_res );
      parameters[ "path_view_bead_model" ] = QString( "%1" ).arg( path_view_bead_model );
      parameters[ "path_open_hydro_res" ] = QString( "%1" ).arg( path_open_hydro_res );
      parameters[ "saxs_options.path_load_saxs_curve" ] = QString( "%1" ).arg( saxs_options.path_load_saxs_curve );
      parameters[ "saxs_options.path_load_gnom" ] = QString( "%1" ).arg( saxs_options.path_load_gnom );
      parameters[ "saxs_options.path_load_prr" ] = QString( "%1" ).arg( saxs_options.path_load_prr );
      parameters[ "asa.hydrate_probe_radius" ] = QString( "%1" ).arg( asa.hydrate_probe_radius );
      parameters[ "asa.hydrate_threshold" ] = QString( "%1" ).arg( asa.hydrate_threshold );
      parameters[ "misc.target_e_density" ] = QString( "%1" ).arg( misc.target_e_density );
      parameters[ "misc.target_volume" ] = QString( "%1" ).arg( misc.target_volume );
      parameters[ "misc.set_target_on_load_pdb" ] = QString( "%1" ).arg( misc.set_target_on_load_pdb );
      parameters[ "misc.equalize_radii" ] = QString( "%1" ).arg( misc.equalize_radii );
      parameters[ "dmd_options.force_chem" ] = QString( "%1" ).arg( dmd_options.force_chem );
      parameters[ "dmd_options.pdb_static_pairs" ] = QString( "%1" ).arg( dmd_options.pdb_static_pairs );
      parameters[ "dmd_options.threshold_pb_pb" ] = QString( "%1" ).arg( dmd_options.threshold_pb_pb );
      parameters[ "dmd_options.threshold_pb_sc" ] = QString( "%1" ).arg( dmd_options.threshold_pb_sc );
      parameters[ "dmd_options.threshold_sc_sc" ] = QString( "%1" ).arg( dmd_options.threshold_sc_sc );
      parameters[ "saxs_options.normalize_by_mw" ] = QString( "%1" ).arg( saxs_options.normalize_by_mw );
      parameters[ "saxs_options.saxs_iq_native_debye" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_debye );
      parameters[ "saxs_options.saxs_iq_native_hybrid" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid );
      parameters[ "saxs_options.saxs_iq_native_hybrid2" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid2 );
      parameters[ "saxs_options.saxs_iq_native_hybrid3" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid3 );
      parameters[ "saxs_options.saxs_iq_native_fast" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_fast );
      parameters[ "saxs_options.saxs_iq_native_fast_compute_pr" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_fast_compute_pr );
      parameters[ "saxs_options.saxs_iq_foxs" ] = QString( "%1" ).arg( saxs_options.saxs_iq_foxs );
      parameters[ "saxs_options.saxs_iq_crysol" ] = QString( "%1" ).arg( saxs_options.saxs_iq_crysol );
      parameters[ "saxs_options.sans_iq_native_debye" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_debye );
      parameters[ "saxs_options.sans_iq_native_hybrid" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid );
      parameters[ "saxs_options.sans_iq_native_hybrid2" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid2 );
      parameters[ "saxs_options.sans_iq_native_hybrid3" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid3 );
      parameters[ "saxs_options.sans_iq_native_fast" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_fast );
      parameters[ "saxs_options.sans_iq_native_fast_compute_pr" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_fast_compute_pr );
      parameters[ "saxs_options.sans_iq_cryson" ] = QString( "%1" ).arg( saxs_options.sans_iq_cryson );
      parameters[ "saxs_options.hybrid2_q_points" ] = QString( "%1" ).arg( saxs_options.hybrid2_q_points );
      parameters[ "saxs_options.iq_ask" ] = QString( "%1" ).arg( saxs_options.iq_ask );
      parameters[ "saxs_options.iq_scale_ask" ] = QString( "%1" ).arg( saxs_options.iq_scale_ask );
      parameters[ "saxs_options.iq_scale_angstrom" ] = QString( "%1" ).arg( saxs_options.iq_scale_angstrom );
      parameters[ "saxs_options.iq_scale_nm" ] = QString( "%1" ).arg( saxs_options.iq_scale_nm );
      parameters[ "saxs_options.sh_max_harmonics" ] = QString( "%1" ).arg( saxs_options.sh_max_harmonics );
      parameters[ "saxs_options.sh_fibonacci_grid_order" ] = QString( "%1" ).arg( saxs_options.sh_fibonacci_grid_order );
      parameters[ "saxs_options.crysol_hydration_shell_contrast" ] = QString( "%1" ).arg( saxs_options.crysol_hydration_shell_contrast );
      parameters[ "saxs_options.crysol_default_load_difference_intensity" ] = QString( "%1" ).arg( saxs_options.crysol_default_load_difference_intensity );
      parameters[ "saxs_options.crysol_version_26" ] = QString( "%1" ).arg( saxs_options.crysol_version_26 );
      parameters[ "saxs_options.crysol_version_3" ] = QString( "%1" ).arg( saxs_options.crysol_version_3 );
      parameters[ "saxs_options.crysol_water_dummy_beads" ] = QString( "%1" ).arg( saxs_options.crysol_water_dummy_beads );
      parameters[ "saxs_options.fast_bin_size" ] = QString( "%1" ).arg( saxs_options.fast_bin_size );
      parameters[ "saxs_options.fast_modulation" ] = QString( "%1" ).arg( saxs_options.fast_modulation );
      parameters[ "saxs_options.compute_saxs_coeff_for_bead_models" ] = QString( "%1" ).arg( saxs_options.compute_saxs_coeff_for_bead_models );
      parameters[ "saxs_options.compute_sans_coeff_for_bead_models" ] = QString( "%1" ).arg( saxs_options.compute_sans_coeff_for_bead_models );
      parameters[ "saxs_options.default_atom_filename" ] = QString( "%1" ).arg( saxs_options.default_atom_filename );
      parameters[ "saxs_options.default_hybrid_filename" ] = QString( "%1" ).arg( saxs_options.default_hybrid_filename );
      parameters[ "saxs_options.default_saxs_filename" ] = QString( "%1" ).arg( saxs_options.default_saxs_filename );
      parameters[ "saxs_options.default_rotamer_filename" ] = QString( "%1" ).arg( saxs_options.default_rotamer_filename );
      parameters[ "saxs_options.steric_clash_distance" ] = QString( "%1" ).arg( saxs_options.steric_clash_distance );
      parameters[ "saxs_options.steric_clash_recheck_distance" ] = QString( "%1" ).arg( saxs_options.steric_clash_recheck_distance );
      parameters[ "saxs_options.disable_iq_scaling" ] = QString( "%1" ).arg( saxs_options.disable_iq_scaling );
      parameters[ "saxs_options.disable_nnls_scaling" ] = QString( "%1" ).arg( saxs_options.disable_nnls_scaling );
      parameters[ "saxs_options.autocorrelate" ] = QString( "%1" ).arg( saxs_options.autocorrelate );
      parameters[ "saxs_options.hybrid_radius_excl_vol" ] = QString( "%1" ).arg( saxs_options.hybrid_radius_excl_vol );
      parameters[ "saxs_options.scale_excl_vol" ] = QString( "%1" ).arg( saxs_options.scale_excl_vol );
      parameters[ "saxs_options.subtract_radius" ] = QString( "%1" ).arg( saxs_options.subtract_radius );
      parameters[ "saxs_options.iqq_scale_minq" ] = QString( "%1" ).arg( saxs_options.iqq_scale_minq );
      parameters[ "saxs_options.iqq_scale_maxq" ] = QString( "%1" ).arg( saxs_options.iqq_scale_maxq );
      parameters[ "saxs_options.iqq_scale_nnls" ] = QString( "%1" ).arg( saxs_options.iqq_scale_nnls );
      parameters[ "saxs_options.iqq_scale_linear_offset" ] = QString( "%1" ).arg( saxs_options.iqq_scale_linear_offset );
      parameters[ "saxs_options.iqq_scale_chi2_fitting" ] = QString( "%1" ).arg( saxs_options.iqq_scale_chi2_fitting );
      parameters[ "saxs_options.iqq_expt_data_contains_variances" ] = QString( "%1" ).arg( saxs_options.iqq_expt_data_contains_variances );
      parameters[ "saxs_options.iqq_ask_target_grid" ] = QString( "%1" ).arg( saxs_options.iqq_ask_target_grid );
      parameters[ "saxs_options.iqq_scale_play" ] = QString( "%1" ).arg( saxs_options.iqq_scale_play );
      parameters[ "saxs_options.swh_excl_vol" ] = QString( "%1" ).arg( saxs_options.swh_excl_vol );
      parameters[ "saxs_options.iqq_default_scaling_target" ] = QString( "%1" ).arg( saxs_options.iqq_default_scaling_target );
      parameters[ "saxs_options.saxs_iq_hybrid_adaptive" ] = QString( "%1" ).arg( saxs_options.saxs_iq_hybrid_adaptive );
      parameters[ "saxs_options.sans_iq_hybrid_adaptive" ] = QString( "%1" ).arg( saxs_options.sans_iq_hybrid_adaptive );
      parameters[ "saxs_options.bead_model_rayleigh" ] = QString( "%1" ).arg( saxs_options.bead_model_rayleigh );
      parameters[ "saxs_options.iqq_log_fitting" ] = QString( "%1" ).arg( saxs_options.iqq_log_fitting );
      parameters[ "saxs_options.iqq_kratky_fit" ] = QString( "%1" ).arg( saxs_options.iqq_kratky_fit );
      parameters[ "saxs_options.iqq_use_atomic_ff" ] = QString( "%1" ).arg( saxs_options.iqq_use_atomic_ff );
      parameters[ "saxs_options.iqq_use_saxs_excl_vol" ] = QString( "%1" ).arg( saxs_options.iqq_use_saxs_excl_vol );
      parameters[ "saxs_options.alt_hydration" ] = QString( "%1" ).arg( saxs_options.alt_hydration );
      parameters[ "saxs_options.xsr_symmop" ] = QString( "%1" ).arg( saxs_options.xsr_symmop );
      parameters[ "saxs_options.xsr_nx" ] = QString( "%1" ).arg( saxs_options.xsr_nx );
      parameters[ "saxs_options.xsr_ny" ] = QString( "%1" ).arg( saxs_options.xsr_ny );
      parameters[ "saxs_options.xsr_griddistance" ] = QString( "%1" ).arg( saxs_options.xsr_griddistance );
      parameters[ "saxs_options.xsr_ncomponents" ] = QString( "%1" ).arg( saxs_options.xsr_ncomponents );
      parameters[ "saxs_options.xsr_compactness_weight" ] = QString( "%1" ).arg( saxs_options.xsr_compactness_weight );
      parameters[ "saxs_options.xsr_looseness_weight" ] = QString( "%1" ).arg( saxs_options.xsr_looseness_weight );
      parameters[ "saxs_options.xsr_temperature" ] = QString( "%1" ).arg( saxs_options.xsr_temperature );
      parameters[ "hydro.zeno_zeno" ] = QString( "%1" ).arg( hydro.zeno_zeno );
      parameters[ "hydro.zeno_interior" ] = QString( "%1" ).arg( hydro.zeno_interior );
      parameters[ "hydro.zeno_surface" ] = QString( "%1" ).arg( hydro.zeno_surface );
      parameters[ "hydro.zeno_zeno_steps" ] = QString( "%1" ).arg( hydro.zeno_zeno_steps );
      parameters[ "hydro.zeno_interior_steps" ] = QString( "%1" ).arg( hydro.zeno_interior_steps );
      parameters[ "hydro.zeno_surface_steps" ] = QString( "%1" ).arg( hydro.zeno_surface_steps );
      parameters[ "hydro.zeno_surface_thickness" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness );
      
      parameters[ "hydro.zeno_surface_thickness_from_rg" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness_from_rg ? "1" : "0" );
      parameters[ "hydro.zeno_surface_thickness_from_rg_a" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness_from_rg_a );
      parameters[ "hydro.zeno_surface_thickness_from_rg_b" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness_from_rg_b );
      parameters[ "hydro.zeno_surface_thickness_from_rg_c" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness_from_rg_c );

      parameters[ "misc.hydro_supc" ] = QString( "%1" ).arg( misc.hydro_supc );
      parameters[ "misc.hydro_zeno" ] = QString( "%1" ).arg( misc.hydro_zeno );
      parameters[ "batch.saxs_search" ] = QString( "%1" ).arg( batch.saxs_search );
      parameters[ "batch.zeno" ] = QString( "%1" ).arg( batch.zeno );

      parameters[ "saxs_options.ignore_errors" ] = QString( "%1" ).arg( saxs_options.ignore_errors );
      parameters[ "saxs_options.trunc_pr_dmax_target" ] = QString( "%1" ).arg( saxs_options.trunc_pr_dmax_target );
      parameters[ "saxs_options.alt_ff" ] = QString( "%1" ).arg( saxs_options.alt_ff );
      parameters[ "saxs_options.crysol_explicit_hydrogens" ] = QString( "%1" ).arg( saxs_options.crysol_explicit_hydrogens );
      parameters[ "saxs_options.use_somo_ff" ] = QString( "%1" ).arg( saxs_options.use_somo_ff );
      parameters[ "saxs_options.five_term_gaussians" ] = QString( "%1" ).arg( saxs_options.five_term_gaussians );
      parameters[ "saxs_options.iq_exact_q" ] = QString( "%1" ).arg( saxs_options.iq_exact_q );
      parameters[ "saxs_options.use_iq_target_ev" ] = QString( "%1" ).arg( saxs_options.use_iq_target_ev );
      parameters[ "saxs_options.set_iq_target_ev_from_vbar" ] = QString( "%1" ).arg( saxs_options.set_iq_target_ev_from_vbar );
      parameters[ "saxs_options.iq_target_ev" ] = QString( "%1" ).arg( saxs_options.iq_target_ev );
      parameters[ "saxs_options.hydration_rev_asa" ] = QString( "%1" ).arg( saxs_options.hydration_rev_asa );
      parameters[ "saxs_options.compute_exponentials" ] = QString( "%1" ).arg( saxs_options.compute_exponentials );
      parameters[ "saxs_options.compute_exponential_terms" ] = QString( "%1" ).arg( saxs_options.compute_exponential_terms );
      parameters[ "saxs_options.dummy_saxs_name" ] = QString( "%1" ).arg( saxs_options.dummy_saxs_name );
      parameters[ "saxs_options.multiply_iq_by_atomic_volume" ] = QString( "%1" ).arg( saxs_options.multiply_iq_by_atomic_volume );
      parameters[ "saxs_options.dummy_atom_pdbs_in_nm" ] = QString( "%1" ).arg( saxs_options.dummy_atom_pdbs_in_nm );
      parameters[ "saxs_options.iq_global_avg_for_bead_models" ] = QString( "%1" ).arg( saxs_options.iq_global_avg_for_bead_models );
      parameters[ "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ] = QString( "%1" ).arg( saxs_options.apply_loaded_sf_repeatedly_to_pdb );
      parameters[ "saxs_options.bead_models_use_var_len_sf" ] = QString( "%1" ).arg( saxs_options.bead_models_use_var_len_sf );
      parameters[ "saxs_options.bead_models_var_len_sf_max" ] = QString( "%1" ).arg( saxs_options.bead_models_var_len_sf_max );
      parameters[ "saxs_options.bead_models_use_gsm_fitting" ] = QString( "%1" ).arg( saxs_options.bead_models_use_gsm_fitting );
      parameters[ "saxs_options.bead_models_use_quick_fitting" ] = QString( "%1" ).arg( saxs_options.bead_models_use_quick_fitting );
      parameters[ "saxs_options.bead_models_use_bead_radius_ev" ] = QString( "%1" ).arg( saxs_options.bead_models_use_bead_radius_ev );
      parameters[ "saxs_options.bead_models_rho0_in_scat_factors" ] = QString( "%1" ).arg( saxs_options.bead_models_rho0_in_scat_factors );
      parameters[ "saxs_options.smooth" ] = QString( "%1" ).arg( saxs_options.smooth );
      parameters[ "saxs_options.ev_exp_mult" ] = QString( "%1" ).arg( saxs_options.ev_exp_mult );
      parameters[ "saxs_options.sastbx_method" ] = QString( "%1" ).arg( saxs_options.sastbx_method );
      parameters[ "saxs_options.saxs_iq_sastbx" ] = QString( "%1" ).arg( saxs_options.saxs_iq_sastbx );
      parameters[ "saxs_options.saxs_iq_native_sh" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_sh );
      parameters[ "saxs_options.sans_iq_native_sh" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_sh );
      parameters[ "saxs_options.alt_sh1" ] = QString( "%1" ).arg( saxs_options.alt_sh1 );
      parameters[ "saxs_options.alt_sh2" ] = QString( "%1" ).arg( saxs_options.alt_sh2 );
      parameters[ "grid.create_nmr_bead_pdb" ] = QString( "%1" ).arg( grid.create_nmr_bead_pdb );
      parameters[ "batch.compute_iq_only_avg" ] = QString( "%1" ).arg( batch.compute_iq_only_avg );
      parameters[ "asa.vvv" ] = QString( "%1" ).arg( asa.vvv );
      parameters[ "asa.vvv_probe_radius" ] = QString( "%1" ).arg( asa.vvv_probe_radius );
      parameters[ "asa.vvv_grid_dR" ] = QString( "%1" ).arg( asa.vvv_grid_dR );
      parameters[ "misc.export_msroll" ] = QString( "%1" ).arg( misc.export_msroll );
      parameters[ "misc.parallel_grpy" ] = QString( "%1" ).arg( misc.parallel_grpy );
      parameters[ "misc.auto_calc_hydro_method" ] = QString( "%1" ).arg( (int) misc.auto_calc_hydro_method );

      parameters[ "saxs_options.qstart" ] = QString( "%1" ).arg( saxs_options.qstart );
      parameters[ "saxs_options.qend" ] = QString( "%1" ).arg( saxs_options.qend );
      parameters[ "saxs_options.guinier_csv" ] = QString( "%1" ).arg( saxs_options.guinier_csv ); // = false;
      parameters[ "saxs_options.guinier_csv_filename" ] = QString( "%1" ).arg( saxs_options.guinier_csv_filename ); // = "guinier";
      parameters[ "saxs_options.qRgmax" ] = QString( "%1" ).arg( saxs_options.qRgmax ); // = 1.3e0;
      parameters[ "saxs_options.pointsmin" ] = QString( "%1" ).arg( saxs_options.pointsmin ); // = 10;
      parameters[ "saxs_options.pointsmax" ] = QString( "%1" ).arg( saxs_options.pointsmax ); // = 100;

      parameters[ "saxs_options.cs_qRgmax" ] = QString( "%1" ).arg( saxs_options.cs_qRgmax );
      parameters[ "saxs_options.cs_qstart" ] = QString( "%1" ).arg( saxs_options.cs_qstart );
      parameters[ "saxs_options.cs_qend" ] = QString( "%1" ).arg( saxs_options.cs_qend );
      parameters[ "saxs_options.conc" ] = QString( "%1" ).arg( saxs_options.conc );
      parameters[ "saxs_options.psv" ] = QString( "%1" ).arg( saxs_options.psv );
      parameters[ "saxs_options.use_cs_psv" ] = QString( "%1" ).arg( saxs_options.use_cs_psv );
      parameters[ "saxs_options.cs_psv" ] = QString( "%1" ).arg( saxs_options.cs_psv );
      parameters[ "saxs_options.I0_exp" ] = QString( "%1" ).arg( saxs_options.I0_exp );
      parameters[ "saxs_options.I0_theo" ] = QString( "%1" ).arg( saxs_options.I0_theo );
      parameters[ "saxs_options.diffusion_len" ] = QString( "%1" ).arg( saxs_options.diffusion_len );
      parameters[ "saxs_options.nucleon_mass" ] = QString( "%1" ).arg( saxs_options.nucleon_mass );
      parameters[ "saxs_options.guinier_outlier_reject" ] = QString( "%1" ).arg( saxs_options.guinier_outlier_reject );
      parameters[ "saxs_options.guinier_outlier_reject_dist" ] = QString( "%1" ).arg( saxs_options.guinier_outlier_reject_dist );
      parameters[ "saxs_options.guinier_use_sd" ] = QString( "%1" ).arg( saxs_options.guinier_use_sd );
      parameters[ "saxs_options.guinier_use_standards" ] = QString( "%1" ).arg( saxs_options.guinier_use_standards );

      parameters[ "saxs_options.cryson_sh_max_harmonics" ] = QString( "%1" ).arg( saxs_options.cryson_sh_max_harmonics ); //            = 15;
      parameters[ "saxs_options.cryson_sh_fibonacci_grid_order" ] = QString( "%1" ).arg( saxs_options.cryson_sh_fibonacci_grid_order ); //     = 17;
      parameters[ "saxs_options.cryson_hydration_shell_contrast" ] = QString( "%1" ).arg( saxs_options.cryson_hydration_shell_contrast ); //    = 0.03f;
      parameters[ "saxs_options.cryson_manual_hs" ] = QString( "%1" ).arg( saxs_options.cryson_manual_hs ); //    = 0.03f;

      parameters[ "advanced_config.temp_dir_threshold_mb" ] = QString( "%1" ).arg( advanced_config.temp_dir_threshold_mb );

      // vectors to write:
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) saxs_options.dummy_saxs_names.size(); i++ )
         {
            qsl_tmp << saxs_options.dummy_saxs_names[ i ];
         }
         parameters[ "saxs_options.dummy_saxs_names" ] = qsl_tmp.join( "\n" );
      }
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) batch.file.size(); i++ )
         {
            qsl_tmp << batch.file[ i ];
         }
         parameters[ "batch.file" ] = qsl_tmp.join( "\n" );
      }
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) save_params.field.size(); i++ )
         {
            qsl_tmp << save_params.field[ i ];
         }
         parameters[ "save_params.field" ] = qsl_tmp.join( "\n" );
      }
      
      for ( map < QString, QString >::iterator it = gparams.begin();
            it != gparams.end();
            it++ )
      {
         parameters[ "gparam:" + it->first ] = it->second;
      }

      {
         QStringList qsl_tmp1;
         QStringList qsl_tmp2;
         QStringList qsl_tmp3;
         for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
         {
            if ( directory_last_access.count( directory_history[ i ] ) )
            { 
               qsl_tmp1 << directory_history[ i ];
               qsl_tmp2 << QString( "%1" ).arg( (unsigned int)directory_last_access[ directory_history[ i ] ].toTime_t() );
               qsl_tmp3 << 
                  ( ( directory_last_filetype.count( directory_history[ i ] ) &&
                      !directory_last_filetype[ directory_history[ i ] ].isEmpty() )
                    ?
                    directory_last_filetype[ directory_history[ i ] ] : "____" );
            }
         }
         if ( qsl_tmp1.size() )
         {
            parameters[ "directory_history"       ] = qsl_tmp1.join( "\n" );
            parameters[ "directory_last_access"   ] = qsl_tmp2.join( "\n" );
            parameters[ "directory_last_filetype" ] = qsl_tmp3.join( "\n" );
         }
      }


      ts << US_Json::compose( parameters );
      f.close();
   }
}

bool US_Hydrodyn::load_config_json ( QString &json )
{
   map < QString, QString > parameters = US_Json::split( json );

   // first set to default parameters
   hard_coded_defaults();

   if ( parameters.count( "replicate_o_r_method_somo" ) ) replicate_o_r_method_somo = parameters[ "replicate_o_r_method_somo" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_overlap" ) ) sidechain_overlap.remove_overlap = parameters[ "sidechain_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "sidechain_overlap.fuse_beads" ) ) sidechain_overlap.fuse_beads = parameters[ "sidechain_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "sidechain_overlap.fuse_beads_percent" ) ) sidechain_overlap.fuse_beads_percent = parameters[ "sidechain_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.remove_hierarch" ) ) sidechain_overlap.remove_hierarch = parameters[ "sidechain_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_hierarch_percent" ) ) sidechain_overlap.remove_hierarch_percent = parameters[ "sidechain_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.remove_sync" ) ) sidechain_overlap.remove_sync = parameters[ "sidechain_overlap.remove_sync" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_sync_percent" ) ) sidechain_overlap.remove_sync_percent = parameters[ "sidechain_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.translate_out" ) ) sidechain_overlap.translate_out = parameters[ "sidechain_overlap.translate_out" ] == "1";
   if ( parameters.count( "sidechain_overlap.show_translate" ) ) sidechain_overlap.show_translate = parameters[ "sidechain_overlap.show_translate" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_overlap" ) ) mainchain_overlap.remove_overlap = parameters[ "mainchain_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "mainchain_overlap.fuse_beads" ) ) mainchain_overlap.fuse_beads = parameters[ "mainchain_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "mainchain_overlap.fuse_beads_percent" ) ) mainchain_overlap.fuse_beads_percent = parameters[ "mainchain_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.remove_hierarch" ) ) mainchain_overlap.remove_hierarch = parameters[ "mainchain_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_hierarch_percent" ) ) mainchain_overlap.remove_hierarch_percent = parameters[ "mainchain_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.remove_sync" ) ) mainchain_overlap.remove_sync = parameters[ "mainchain_overlap.remove_sync" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_sync_percent" ) ) mainchain_overlap.remove_sync_percent = parameters[ "mainchain_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.translate_out" ) ) mainchain_overlap.translate_out = parameters[ "mainchain_overlap.translate_out" ] == "1";
   if ( parameters.count( "mainchain_overlap.show_translate" ) ) mainchain_overlap.show_translate = parameters[ "mainchain_overlap.show_translate" ] == "1";
   if ( parameters.count( "buried_overlap.remove_overlap" ) ) buried_overlap.remove_overlap = parameters[ "buried_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "buried_overlap.fuse_beads" ) ) buried_overlap.fuse_beads = parameters[ "buried_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "buried_overlap.fuse_beads_percent" ) ) buried_overlap.fuse_beads_percent = parameters[ "buried_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.remove_hierarch" ) ) buried_overlap.remove_hierarch = parameters[ "buried_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "buried_overlap.remove_hierarch_percent" ) ) buried_overlap.remove_hierarch_percent = parameters[ "buried_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.remove_sync" ) ) buried_overlap.remove_sync = parameters[ "buried_overlap.remove_sync" ] == "1";
   if ( parameters.count( "buried_overlap.remove_sync_percent" ) ) buried_overlap.remove_sync_percent = parameters[ "buried_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.translate_out" ) ) buried_overlap.translate_out = parameters[ "buried_overlap.translate_out" ] == "1";
   if ( parameters.count( "buried_overlap.show_translate" ) ) buried_overlap.show_translate = parameters[ "buried_overlap.show_translate" ] == "1";
   if ( parameters.count( "replicate_o_r_method_grid" ) ) replicate_o_r_method_grid = parameters[ "replicate_o_r_method_grid" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_overlap" ) ) grid_exposed_overlap.remove_overlap = parameters[ "grid_exposed_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.fuse_beads" ) ) grid_exposed_overlap.fuse_beads = parameters[ "grid_exposed_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.fuse_beads_percent" ) ) grid_exposed_overlap.fuse_beads_percent = parameters[ "grid_exposed_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.remove_hierarch" ) ) grid_exposed_overlap.remove_hierarch = parameters[ "grid_exposed_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_hierarch_percent" ) ) grid_exposed_overlap.remove_hierarch_percent = parameters[ "grid_exposed_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.remove_sync" ) ) grid_exposed_overlap.remove_sync = parameters[ "grid_exposed_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_sync_percent" ) ) grid_exposed_overlap.remove_sync_percent = parameters[ "grid_exposed_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.translate_out" ) ) grid_exposed_overlap.translate_out = parameters[ "grid_exposed_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.show_translate" ) ) grid_exposed_overlap.show_translate = parameters[ "grid_exposed_overlap.show_translate" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_overlap" ) ) grid_buried_overlap.remove_overlap = parameters[ "grid_buried_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_buried_overlap.fuse_beads" ) ) grid_buried_overlap.fuse_beads = parameters[ "grid_buried_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_buried_overlap.fuse_beads_percent" ) ) grid_buried_overlap.fuse_beads_percent = parameters[ "grid_buried_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.remove_hierarch" ) ) grid_buried_overlap.remove_hierarch = parameters[ "grid_buried_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_hierarch_percent" ) ) grid_buried_overlap.remove_hierarch_percent = parameters[ "grid_buried_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.remove_sync" ) ) grid_buried_overlap.remove_sync = parameters[ "grid_buried_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_sync_percent" ) ) grid_buried_overlap.remove_sync_percent = parameters[ "grid_buried_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.translate_out" ) ) grid_buried_overlap.translate_out = parameters[ "grid_buried_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_buried_overlap.show_translate" ) ) grid_buried_overlap.show_translate = parameters[ "grid_buried_overlap.show_translate" ] == "1";
   if ( parameters.count( "grid_overlap.remove_overlap" ) ) grid_overlap.remove_overlap = parameters[ "grid_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_overlap.fuse_beads" ) ) grid_overlap.fuse_beads = parameters[ "grid_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_overlap.fuse_beads_percent" ) ) grid_overlap.fuse_beads_percent = parameters[ "grid_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.remove_hierarch" ) ) grid_overlap.remove_hierarch = parameters[ "grid_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_overlap.remove_hierarch_percent" ) ) grid_overlap.remove_hierarch_percent = parameters[ "grid_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.remove_sync" ) ) grid_overlap.remove_sync = parameters[ "grid_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_overlap.remove_sync_percent" ) ) grid_overlap.remove_sync_percent = parameters[ "grid_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.translate_out" ) ) grid_overlap.translate_out = parameters[ "grid_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_overlap.show_translate" ) ) grid_overlap.show_translate = parameters[ "grid_overlap.show_translate" ] == "1";
   if ( parameters.count( "overlap_tolerance" ) ) overlap_tolerance = parameters[ "overlap_tolerance" ].toDouble();
   if ( parameters.count( "bead_output.output" ) ) bead_output.output = parameters[ "bead_output.output" ].toInt();
   if ( parameters.count( "bead_output.sequence" ) ) bead_output.sequence = parameters[ "bead_output.sequence" ].toInt();
   if ( parameters.count( "bead_output.correspondence" ) ) bead_output.correspondence = parameters[ "bead_output.correspondence" ] == "1";
   if ( parameters.count( "asa.probe_radius" ) ) asa.probe_radius = parameters[ "asa.probe_radius" ].toFloat();
   if ( parameters.count( "asa.probe_recheck_radius" ) ) asa.probe_recheck_radius = parameters[ "asa.probe_recheck_radius" ].toFloat();
   if ( parameters.count( "asa.threshold" ) ) asa.threshold = parameters[ "asa.threshold" ].toFloat();
   if ( parameters.count( "asa.threshold_percent" ) ) asa.threshold_percent = parameters[ "asa.threshold_percent" ].toFloat();
   if ( parameters.count( "asa.vdw_grpy_probe_radius" ) ) asa.vdw_grpy_probe_radius = parameters[ "asa.vdw_grpy_probe_radius" ].toFloat();
   if ( parameters.count( "asa.vdw_grpy_threshold_percent" ) ) asa.vdw_grpy_threshold_percent = parameters[ "asa.vdw_grpy_threshold_percent" ].toFloat();
   if ( parameters.count( "asa.grid_threshold" ) ) asa.grid_threshold = parameters[ "asa.grid_threshold" ].toFloat();
   if ( parameters.count( "asa.grid_threshold_percent" ) ) asa.grid_threshold_percent = parameters[ "asa.grid_threshold_percent" ].toFloat();
   if ( parameters.count( "asa.calculation" ) ) asa.calculation = parameters[ "asa.calculation" ] == "1";
   if ( parameters.count( "asa.recheck_beads" ) ) asa.recheck_beads = parameters[ "asa.recheck_beads" ] == "1";
   if ( parameters.count( "asa.method" ) ) asa.method = parameters[ "asa.method" ].toInt();
   if ( parameters.count( "asa.asab1_step" ) ) asa.asab1_step = parameters[ "asa.asab1_step" ].toFloat();
   if ( parameters.count( "grid.cubic" ) ) grid.cubic = parameters[ "grid.cubic" ] == "1";
   if ( parameters.count( "grid.hydrate" ) ) grid.hydrate = parameters[ "grid.hydrate" ] == "1";
   if ( parameters.count( "grid.center" ) ) grid.center = parameters[ "grid.center" ].toInt();
   if ( parameters.count( "grid.tangency" ) ) grid.tangency = parameters[ "grid.tangency" ] == "1";
   if ( parameters.count( "grid.cube_side" ) ) grid.cube_side = parameters[ "grid.cube_side" ].toDouble();
   if ( parameters.count( "grid.enable_asa" ) ) grid.enable_asa = parameters[ "grid.enable_asa" ] == "1";
   if ( parameters.count( "misc.hydrovol" ) ) misc.hydrovol = parameters[ "misc.hydrovol" ].toDouble();
   if ( parameters.count( "misc.compute_vbar" ) ) misc.compute_vbar = parameters[ "misc.compute_vbar" ] == "1";
   if ( parameters.count( "misc.vbar" ) ) misc.vbar = parameters[ "misc.vbar" ].toDouble();
   if ( parameters.count( "misc.vbar_temperature" ) ) misc.vbar_temperature = parameters[ "misc.vbar_temperature" ].toDouble();
   if ( parameters.count( "misc.pb_rule_on" ) ) misc.pb_rule_on = parameters[ "misc.pb_rule_on" ] == "1";
   if ( parameters.count( "misc.avg_radius" ) ) misc.avg_radius = parameters[ "misc.avg_radius" ].toDouble();
   if ( parameters.count( "misc.avg_mass" ) ) misc.avg_mass = parameters[ "misc.avg_mass" ].toDouble();
   if ( parameters.count( "misc.avg_num_elect" ) ) misc.avg_num_elect = parameters[ "misc.avg_num_elect" ].toDouble();
   if ( parameters.count( "misc.avg_protons" ) ) misc.avg_protons = parameters[ "misc.avg_protons" ].toDouble();
   if ( parameters.count( "misc.avg_hydration" ) ) misc.avg_hydration = parameters[ "misc.avg_hydration" ].toDouble();
   if ( parameters.count( "misc.avg_volume" ) ) misc.avg_volume = parameters[ "misc.avg_volume" ].toDouble();
   if ( parameters.count( "misc.avg_vbar" ) ) misc.avg_vbar = parameters[ "misc.avg_vbar" ].toDouble();
   if ( parameters.count( "hydro.unit" ) ) hydro.unit = parameters[ "hydro.unit" ].toInt();
   if ( parameters.count( "hydro.solvent_name" ) ) hydro.solvent_name = parameters[ "hydro.solvent_name" ];
   if ( parameters.count( "hydro.solvent_acronym" ) ) hydro.solvent_acronym = parameters[ "hydro.solvent_acronym" ];
   if ( parameters.count( "hydro.temperature" ) ) hydro.temperature = parameters[ "hydro.temperature" ].toDouble();
   if ( parameters.count( "hydro.pH" ) ) hydro.pH = parameters[ "hydro.pH" ].toDouble();
   if ( parameters.count( "hydro.solvent_viscosity" ) ) hydro.solvent_viscosity = parameters[ "hydro.solvent_viscosity" ].toDouble();
   if ( parameters.count( "hydro.solvent_density" ) ) hydro.solvent_density = parameters[ "hydro.solvent_density" ].toDouble();
   if ( parameters.count( "hydro.reference_system" ) ) hydro.reference_system = parameters[ "hydro.reference_system" ] == "1";
   if ( parameters.count( "hydro.boundary_cond" ) ) hydro.boundary_cond = parameters[ "hydro.boundary_cond" ] == "1";
   if ( parameters.count( "hydro.volume_correction" ) ) hydro.volume_correction = parameters[ "hydro.volume_correction" ] == "1";
   if ( parameters.count( "hydro.use_avg_for_volume" ) ) hydro.use_avg_for_volume = parameters[ "hydro.use_avg_for_volume" ] == "1";
   if ( parameters.count( "hydro.volume" ) ) hydro.volume = parameters[ "hydro.volume" ].toDouble();
   if ( parameters.count( "hydro.mass_correction" ) ) hydro.mass_correction = parameters[ "hydro.mass_correction" ] == "1";
   if ( parameters.count( "hydro.mass" ) ) hydro.mass = parameters[ "hydro.mass" ].toDouble();
   if ( parameters.count( "hydro.bead_inclusion" ) ) hydro.bead_inclusion = parameters[ "hydro.bead_inclusion" ] == "1";
   if ( parameters.count( "hydro.grpy_bead_inclusion" ) ) hydro.grpy_bead_inclusion = parameters[ "hydro.grpy_bead_inclusion" ] == "1";
   if ( parameters.count( "hydro.rotational" ) ) hydro.rotational = parameters[ "hydro.rotational" ] == "1";
   if ( parameters.count( "hydro.viscosity" ) ) hydro.viscosity = parameters[ "hydro.viscosity" ] == "1";
   if ( parameters.count( "hydro.overlap_cutoff" ) ) hydro.overlap_cutoff = parameters[ "hydro.overlap_cutoff" ] == "1";
   if ( parameters.count( "hydro.overlap" ) ) hydro.overlap = parameters[ "hydro.overlap" ].toDouble();
   if ( parameters.count( "pdb_vis.visualization" ) ) pdb_vis.visualization = parameters[ "pdb_vis.visualization" ].toInt();
   if ( parameters.count( "pdb_vis.filename" ) ) pdb_vis.filename = parameters[ "pdb_vis.filename" ];
   if ( parameters.count( "pdb_parse.skip_hydrogen" ) ) pdb_parse.skip_hydrogen = parameters[ "pdb_parse.skip_hydrogen" ] == "1";
   if ( parameters.count( "pdb_parse.skip_water" ) ) pdb_parse.skip_water = parameters[ "pdb_parse.skip_water" ] == "1";
   if ( parameters.count( "pdb_parse.alternate" ) ) pdb_parse.alternate = parameters[ "pdb_parse.alternate" ] == "1";
   if ( parameters.count( "pdb_parse.find_sh" ) ) pdb_parse.find_sh = parameters[ "pdb_parse.find_sh" ] == "1";
   if ( parameters.count( "pdb_parse.missing_residues" ) ) pdb_parse.missing_residues = parameters[ "pdb_parse.missing_residues" ].toInt();
   if ( parameters.count( "pdb_parse.missing_atoms" ) ) pdb_parse.missing_atoms = parameters[ "pdb_parse.missing_atoms" ].toInt();
   if ( parameters.count( "saxs_options.water_e_density" ) ) saxs_options.water_e_density = parameters[ "saxs_options.water_e_density" ].toFloat();
   if ( parameters.count( "saxs_options.h_scat_len" ) ) saxs_options.h_scat_len = parameters[ "saxs_options.h_scat_len" ].toFloat();
   if ( parameters.count( "saxs_options.d_scat_len" ) ) saxs_options.d_scat_len = parameters[ "saxs_options.d_scat_len" ].toFloat();
   if ( parameters.count( "saxs_options.h2o_scat_len_dens" ) ) saxs_options.h2o_scat_len_dens = parameters[ "saxs_options.h2o_scat_len_dens" ].toFloat();
   if ( parameters.count( "saxs_options.d2o_scat_len_dens" ) ) saxs_options.d2o_scat_len_dens = parameters[ "saxs_options.d2o_scat_len_dens" ].toFloat();
   if ( parameters.count( "saxs_options.d2o_conc" ) ) saxs_options.d2o_conc = parameters[ "saxs_options.d2o_conc" ].toFloat();
   if ( parameters.count( "saxs_options.frac_of_exch_pep" ) ) saxs_options.frac_of_exch_pep = parameters[ "saxs_options.frac_of_exch_pep" ].toFloat();
   if ( parameters.count( "saxs_options.wavelength" ) ) saxs_options.wavelength = parameters[ "saxs_options.wavelength" ].toFloat();
   if ( parameters.count( "saxs_options.start_angle" ) ) saxs_options.start_angle = parameters[ "saxs_options.start_angle" ].toFloat();
   if ( parameters.count( "saxs_options.end_angle" ) ) saxs_options.end_angle = parameters[ "saxs_options.end_angle" ].toFloat();
   if ( parameters.count( "saxs_options.delta_angle" ) ) saxs_options.delta_angle = parameters[ "saxs_options.delta_angle" ].toFloat();
   if ( parameters.count( "saxs_options.max_size" ) ) saxs_options.max_size = parameters[ "saxs_options.max_size" ].toFloat();
   if ( parameters.count( "saxs_options.bin_size" ) ) saxs_options.bin_size = parameters[ "saxs_options.bin_size" ].toFloat();
   if ( parameters.count( "saxs_options.hydrate_pdb" ) ) saxs_options.hydrate_pdb = parameters[ "saxs_options.hydrate_pdb" ] == "1";
   if ( parameters.count( "saxs_options.curve" ) ) saxs_options.curve = parameters[ "saxs_options.curve" ].toInt();
   if ( parameters.count( "saxs_options.saxs_sans" ) ) saxs_options.saxs_sans = parameters[ "saxs_options.saxs_sans" ].toInt();
   if ( parameters.count( "bd_options.threshold_pb_pb" ) ) bd_options.threshold_pb_pb = parameters[ "bd_options.threshold_pb_pb" ].toFloat();
   if ( parameters.count( "bd_options.threshold_pb_sc" ) ) bd_options.threshold_pb_sc = parameters[ "bd_options.threshold_pb_sc" ].toFloat();
   if ( parameters.count( "bd_options.threshold_sc_sc" ) ) bd_options.threshold_sc_sc = parameters[ "bd_options.threshold_sc_sc" ].toFloat();
   if ( parameters.count( "bd_options.do_rr" ) ) bd_options.do_rr = parameters[ "bd_options.do_rr" ] == "1";
   if ( parameters.count( "bd_options.force_chem" ) ) bd_options.force_chem = parameters[ "bd_options.force_chem" ] == "1";
   if ( parameters.count( "bd_options.bead_size_type" ) ) bd_options.bead_size_type = parameters[ "bd_options.bead_size_type" ].toInt();
   if ( parameters.count( "bd_options.show_pdb" ) ) bd_options.show_pdb = parameters[ "bd_options.show_pdb" ] == "1";
   if ( parameters.count( "bd_options.run_browflex" ) ) bd_options.run_browflex = parameters[ "bd_options.run_browflex" ] == "1";
   if ( parameters.count( "bd_options.tprev" ) ) bd_options.tprev = parameters[ "bd_options.tprev" ].toDouble();
   if ( parameters.count( "bd_options.ttraj" ) ) bd_options.ttraj = parameters[ "bd_options.ttraj" ].toDouble();
   if ( parameters.count( "bd_options.deltat" ) ) bd_options.deltat = parameters[ "bd_options.deltat" ].toDouble();
   if ( parameters.count( "bd_options.npadif" ) ) bd_options.npadif = parameters[ "bd_options.npadif" ].toInt();
   if ( parameters.count( "bd_options.nconf" ) ) bd_options.nconf = parameters[ "bd_options.nconf" ].toInt();
   if ( parameters.count( "bd_options.inter" ) ) bd_options.inter = parameters[ "bd_options.inter" ].toInt();
   if ( parameters.count( "bd_options.iorder" ) ) bd_options.iorder = parameters[ "bd_options.iorder" ].toInt();
   if ( parameters.count( "bd_options.iseed" ) ) bd_options.iseed = parameters[ "bd_options.iseed" ].toInt();
   if ( parameters.count( "bd_options.icdm" ) ) bd_options.icdm = parameters[ "bd_options.icdm" ].toInt();
   if ( parameters.count( "bd_options.chem_pb_pb_bond_type" ) ) bd_options.chem_pb_pb_bond_type = parameters[ "bd_options.chem_pb_pb_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_force_constant" ) ) bd_options.compute_chem_pb_pb_force_constant = parameters[ "bd_options.compute_chem_pb_pb_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_force_constant" ) ) bd_options.chem_pb_pb_force_constant = parameters[ "bd_options.chem_pb_pb_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_equilibrium_dist" ) ) bd_options.compute_chem_pb_pb_equilibrium_dist = parameters[ "bd_options.compute_chem_pb_pb_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_equilibrium_dist" ) ) bd_options.chem_pb_pb_equilibrium_dist = parameters[ "bd_options.chem_pb_pb_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_max_elong" ) ) bd_options.compute_chem_pb_pb_max_elong = parameters[ "bd_options.compute_chem_pb_pb_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_max_elong" ) ) bd_options.chem_pb_pb_max_elong = parameters[ "bd_options.chem_pb_pb_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.chem_pb_sc_bond_type" ) ) bd_options.chem_pb_sc_bond_type = parameters[ "bd_options.chem_pb_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_force_constant" ) ) bd_options.compute_chem_pb_sc_force_constant = parameters[ "bd_options.compute_chem_pb_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_force_constant" ) ) bd_options.chem_pb_sc_force_constant = parameters[ "bd_options.chem_pb_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_equilibrium_dist" ) ) bd_options.compute_chem_pb_sc_equilibrium_dist = parameters[ "bd_options.compute_chem_pb_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_equilibrium_dist" ) ) bd_options.chem_pb_sc_equilibrium_dist = parameters[ "bd_options.chem_pb_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_max_elong" ) ) bd_options.compute_chem_pb_sc_max_elong = parameters[ "bd_options.compute_chem_pb_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_max_elong" ) ) bd_options.chem_pb_sc_max_elong = parameters[ "bd_options.chem_pb_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.chem_sc_sc_bond_type" ) ) bd_options.chem_sc_sc_bond_type = parameters[ "bd_options.chem_sc_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_force_constant" ) ) bd_options.compute_chem_sc_sc_force_constant = parameters[ "bd_options.compute_chem_sc_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_force_constant" ) ) bd_options.chem_sc_sc_force_constant = parameters[ "bd_options.chem_sc_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_equilibrium_dist" ) ) bd_options.compute_chem_sc_sc_equilibrium_dist = parameters[ "bd_options.compute_chem_sc_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_equilibrium_dist" ) ) bd_options.chem_sc_sc_equilibrium_dist = parameters[ "bd_options.chem_sc_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_max_elong" ) ) bd_options.compute_chem_sc_sc_max_elong = parameters[ "bd_options.compute_chem_sc_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_max_elong" ) ) bd_options.chem_sc_sc_max_elong = parameters[ "bd_options.chem_sc_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.pb_pb_bond_type" ) ) bd_options.pb_pb_bond_type = parameters[ "bd_options.pb_pb_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_pb_pb_force_constant" ) ) bd_options.compute_pb_pb_force_constant = parameters[ "bd_options.compute_pb_pb_force_constant" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_force_constant" ) ) bd_options.pb_pb_force_constant = parameters[ "bd_options.pb_pb_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_pb_equilibrium_dist" ) ) bd_options.compute_pb_pb_equilibrium_dist = parameters[ "bd_options.compute_pb_pb_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_equilibrium_dist" ) ) bd_options.pb_pb_equilibrium_dist = parameters[ "bd_options.pb_pb_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_pb_max_elong" ) ) bd_options.compute_pb_pb_max_elong = parameters[ "bd_options.compute_pb_pb_max_elong" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_max_elong" ) ) bd_options.pb_pb_max_elong = parameters[ "bd_options.pb_pb_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.pb_sc_bond_type" ) ) bd_options.pb_sc_bond_type = parameters[ "bd_options.pb_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_pb_sc_force_constant" ) ) bd_options.compute_pb_sc_force_constant = parameters[ "bd_options.compute_pb_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_force_constant" ) ) bd_options.pb_sc_force_constant = parameters[ "bd_options.pb_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_sc_equilibrium_dist" ) ) bd_options.compute_pb_sc_equilibrium_dist = parameters[ "bd_options.compute_pb_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_equilibrium_dist" ) ) bd_options.pb_sc_equilibrium_dist = parameters[ "bd_options.pb_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_sc_max_elong" ) ) bd_options.compute_pb_sc_max_elong = parameters[ "bd_options.compute_pb_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_max_elong" ) ) bd_options.pb_sc_max_elong = parameters[ "bd_options.pb_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.sc_sc_bond_type" ) ) bd_options.sc_sc_bond_type = parameters[ "bd_options.sc_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_sc_sc_force_constant" ) ) bd_options.compute_sc_sc_force_constant = parameters[ "bd_options.compute_sc_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_force_constant" ) ) bd_options.sc_sc_force_constant = parameters[ "bd_options.sc_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_sc_sc_equilibrium_dist" ) ) bd_options.compute_sc_sc_equilibrium_dist = parameters[ "bd_options.compute_sc_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_equilibrium_dist" ) ) bd_options.sc_sc_equilibrium_dist = parameters[ "bd_options.sc_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_sc_sc_max_elong" ) ) bd_options.compute_sc_sc_max_elong = parameters[ "bd_options.compute_sc_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_max_elong" ) ) bd_options.sc_sc_max_elong = parameters[ "bd_options.sc_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.nmol" ) ) bd_options.nmol = parameters[ "bd_options.nmol" ].toInt();
   if ( parameters.count( "anaflex_options.run_anaflex" ) ) anaflex_options.run_anaflex = parameters[ "anaflex_options.run_anaflex" ] == "1";
   if ( parameters.count( "anaflex_options.nfrec" ) ) anaflex_options.nfrec = parameters[ "anaflex_options.nfrec" ].toInt();
   if ( parameters.count( "anaflex_options.instprofiles" ) ) anaflex_options.instprofiles = parameters[ "anaflex_options.instprofiles" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1" ) ) anaflex_options.run_mode_1 = parameters[ "anaflex_options.run_mode_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_1" ) ) anaflex_options.run_mode_1_1 = parameters[ "anaflex_options.run_mode_1_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_2" ) ) anaflex_options.run_mode_1_2 = parameters[ "anaflex_options.run_mode_1_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_3" ) ) anaflex_options.run_mode_1_3 = parameters[ "anaflex_options.run_mode_1_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_4" ) ) anaflex_options.run_mode_1_4 = parameters[ "anaflex_options.run_mode_1_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_5" ) ) anaflex_options.run_mode_1_5 = parameters[ "anaflex_options.run_mode_1_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_7" ) ) anaflex_options.run_mode_1_7 = parameters[ "anaflex_options.run_mode_1_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_8" ) ) anaflex_options.run_mode_1_8 = parameters[ "anaflex_options.run_mode_1_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_12" ) ) anaflex_options.run_mode_1_12 = parameters[ "anaflex_options.run_mode_1_12" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_13" ) ) anaflex_options.run_mode_1_13 = parameters[ "anaflex_options.run_mode_1_13" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_14" ) ) anaflex_options.run_mode_1_14 = parameters[ "anaflex_options.run_mode_1_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_18" ) ) anaflex_options.run_mode_1_18 = parameters[ "anaflex_options.run_mode_1_18" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_20" ) ) anaflex_options.run_mode_1_20 = parameters[ "anaflex_options.run_mode_1_20" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_24" ) ) anaflex_options.run_mode_1_24 = parameters[ "anaflex_options.run_mode_1_24" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2" ) ) anaflex_options.run_mode_2 = parameters[ "anaflex_options.run_mode_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_1" ) ) anaflex_options.run_mode_2_1 = parameters[ "anaflex_options.run_mode_2_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_2" ) ) anaflex_options.run_mode_2_2 = parameters[ "anaflex_options.run_mode_2_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_3" ) ) anaflex_options.run_mode_2_3 = parameters[ "anaflex_options.run_mode_2_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_4" ) ) anaflex_options.run_mode_2_4 = parameters[ "anaflex_options.run_mode_2_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_5" ) ) anaflex_options.run_mode_2_5 = parameters[ "anaflex_options.run_mode_2_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_7" ) ) anaflex_options.run_mode_2_7 = parameters[ "anaflex_options.run_mode_2_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_8" ) ) anaflex_options.run_mode_2_8 = parameters[ "anaflex_options.run_mode_2_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_12" ) ) anaflex_options.run_mode_2_12 = parameters[ "anaflex_options.run_mode_2_12" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_13" ) ) anaflex_options.run_mode_2_13 = parameters[ "anaflex_options.run_mode_2_13" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_14" ) ) anaflex_options.run_mode_2_14 = parameters[ "anaflex_options.run_mode_2_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_18" ) ) anaflex_options.run_mode_2_18 = parameters[ "anaflex_options.run_mode_2_18" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_20" ) ) anaflex_options.run_mode_2_20 = parameters[ "anaflex_options.run_mode_2_20" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_24" ) ) anaflex_options.run_mode_2_24 = parameters[ "anaflex_options.run_mode_2_24" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3" ) ) anaflex_options.run_mode_3 = parameters[ "anaflex_options.run_mode_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_1" ) ) anaflex_options.run_mode_3_1 = parameters[ "anaflex_options.run_mode_3_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_5" ) ) anaflex_options.run_mode_3_5 = parameters[ "anaflex_options.run_mode_3_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_9" ) ) anaflex_options.run_mode_3_9 = parameters[ "anaflex_options.run_mode_3_9" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_10" ) ) anaflex_options.run_mode_3_10 = parameters[ "anaflex_options.run_mode_3_10" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_14" ) ) anaflex_options.run_mode_3_14 = parameters[ "anaflex_options.run_mode_3_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_15" ) ) anaflex_options.run_mode_3_15 = parameters[ "anaflex_options.run_mode_3_15" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_16" ) ) anaflex_options.run_mode_3_16 = parameters[ "anaflex_options.run_mode_3_16" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4" ) ) anaflex_options.run_mode_4 = parameters[ "anaflex_options.run_mode_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_1" ) ) anaflex_options.run_mode_4_1 = parameters[ "anaflex_options.run_mode_4_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_6" ) ) anaflex_options.run_mode_4_6 = parameters[ "anaflex_options.run_mode_4_6" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_7" ) ) anaflex_options.run_mode_4_7 = parameters[ "anaflex_options.run_mode_4_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_8" ) ) anaflex_options.run_mode_4_8 = parameters[ "anaflex_options.run_mode_4_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_9" ) ) anaflex_options.run_mode_9 = parameters[ "anaflex_options.run_mode_9" ] == "1";
   if ( parameters.count( "anaflex_options.ntimc" ) ) anaflex_options.ntimc = parameters[ "anaflex_options.ntimc" ].toInt();
   if ( parameters.count( "anaflex_options.tmax" ) ) anaflex_options.tmax = parameters[ "anaflex_options.tmax" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_5_iii" ) ) anaflex_options.run_mode_3_5_iii = parameters[ "anaflex_options.run_mode_3_5_iii" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_5_jjj" ) ) anaflex_options.run_mode_3_5_jjj = parameters[ "anaflex_options.run_mode_3_5_jjj" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_10_theta" ) ) anaflex_options.run_mode_3_10_theta = parameters[ "anaflex_options.run_mode_3_10_theta" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_10_refractive_index" ) ) anaflex_options.run_mode_3_10_refractive_index = parameters[ "anaflex_options.run_mode_3_10_refractive_index" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_10_lambda" ) ) anaflex_options.run_mode_3_10_lambda = parameters[ "anaflex_options.run_mode_3_10_lambda" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_14_iii" ) ) anaflex_options.run_mode_3_14_iii = parameters[ "anaflex_options.run_mode_3_14_iii" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_14_jjj" ) ) anaflex_options.run_mode_3_14_jjj = parameters[ "anaflex_options.run_mode_3_14_jjj" ].toInt();
   if ( parameters.count( "batch.missing_atoms" ) ) batch.missing_atoms = parameters[ "batch.missing_atoms" ].toInt();
   if ( parameters.count( "batch.missing_residues" ) ) batch.missing_residues = parameters[ "batch.missing_residues" ].toInt();
   if ( parameters.count( "batch.somo" ) ) batch.somo = parameters[ "batch.somo" ] == "1";
   if ( parameters.count( "batch.grid" ) ) batch.grid = parameters[ "batch.grid" ] == "1";
   if ( parameters.count( "batch.hydro" ) ) batch.hydro = parameters[ "batch.hydro" ] == "1";
   if ( parameters.count( "batch.avg_hydro" ) ) batch.avg_hydro = parameters[ "batch.avg_hydro" ] == "1";
   if ( parameters.count( "batch.avg_hydro_name" ) ) batch.avg_hydro_name = parameters[ "batch.avg_hydro_name" ];
   if ( parameters.count( "batch.height" ) ) batch.height = parameters[ "batch.height" ].toInt();
   if ( parameters.count( "batch.width" ) ) batch.width = parameters[ "batch.width" ].toInt();
   if ( parameters.count( "path_load_pdb" ) ) path_load_pdb = parameters[ "path_load_pdb" ];
   if ( parameters.count( "path_view_pdb" ) ) path_view_pdb = parameters[ "path_view_pdb" ];
   if ( parameters.count( "path_load_bead_model" ) ) path_load_bead_model = parameters[ "path_load_bead_model" ];
   if ( parameters.count( "path_view_asa_res" ) ) path_view_asa_res = parameters[ "path_view_asa_res" ];
   if ( parameters.count( "path_view_bead_model" ) ) path_view_bead_model = parameters[ "path_view_bead_model" ];
   if ( parameters.count( "path_open_hydro_res" ) ) path_open_hydro_res = parameters[ "path_open_hydro_res" ];
   if ( parameters.count( "saxs_options.path_load_saxs_curve" ) ) saxs_options.path_load_saxs_curve = parameters[ "saxs_options.path_load_saxs_curve" ];
   if ( parameters.count( "saxs_options.path_load_gnom" ) ) saxs_options.path_load_gnom = parameters[ "saxs_options.path_load_gnom" ];
   if ( parameters.count( "saxs_options.path_load_prr" ) ) saxs_options.path_load_prr = parameters[ "saxs_options.path_load_prr" ];
   if ( parameters.count( "asa.hydrate_probe_radius" ) ) asa.hydrate_probe_radius = parameters[ "asa.hydrate_probe_radius" ].toFloat();
   if ( parameters.count( "asa.hydrate_threshold" ) ) asa.hydrate_threshold = parameters[ "asa.hydrate_threshold" ].toFloat();
   if ( parameters.count( "misc.target_e_density" ) ) misc.target_e_density = parameters[ "misc.target_e_density" ].toDouble();
   if ( parameters.count( "misc.target_volume" ) ) misc.target_volume = parameters[ "misc.target_volume" ].toDouble();
   if ( parameters.count( "misc.set_target_on_load_pdb" ) ) misc.set_target_on_load_pdb = parameters[ "misc.set_target_on_load_pdb" ] == "1";
   if ( parameters.count( "misc.equalize_radii" ) ) misc.equalize_radii = parameters[ "misc.equalize_radii" ] == "1";
   if ( parameters.count( "dmd_options.force_chem" ) ) dmd_options.force_chem = parameters[ "dmd_options.force_chem" ] == "1";
   if ( parameters.count( "dmd_options.pdb_static_pairs" ) ) dmd_options.pdb_static_pairs = parameters[ "dmd_options.pdb_static_pairs" ] == "1";
   if ( parameters.count( "dmd_options.threshold_pb_pb" ) ) dmd_options.threshold_pb_pb = parameters[ "dmd_options.threshold_pb_pb" ].toFloat();
   if ( parameters.count( "dmd_options.threshold_pb_sc" ) ) dmd_options.threshold_pb_sc = parameters[ "dmd_options.threshold_pb_sc" ].toFloat();
   if ( parameters.count( "dmd_options.threshold_sc_sc" ) ) dmd_options.threshold_sc_sc = parameters[ "dmd_options.threshold_sc_sc" ].toFloat();
   if ( parameters.count( "saxs_options.normalize_by_mw" ) ) saxs_options.normalize_by_mw = parameters[ "saxs_options.normalize_by_mw" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_debye" ) ) saxs_options.saxs_iq_native_debye = parameters[ "saxs_options.saxs_iq_native_debye" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid" ) ) saxs_options.saxs_iq_native_hybrid = parameters[ "saxs_options.saxs_iq_native_hybrid" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid2" ) ) saxs_options.saxs_iq_native_hybrid2 = parameters[ "saxs_options.saxs_iq_native_hybrid2" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid3" ) ) saxs_options.saxs_iq_native_hybrid3 = parameters[ "saxs_options.saxs_iq_native_hybrid3" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_fast" ) ) saxs_options.saxs_iq_native_fast = parameters[ "saxs_options.saxs_iq_native_fast" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_fast_compute_pr" ) ) saxs_options.saxs_iq_native_fast_compute_pr = parameters[ "saxs_options.saxs_iq_native_fast_compute_pr" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_foxs" ) ) saxs_options.saxs_iq_foxs = parameters[ "saxs_options.saxs_iq_foxs" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_crysol" ) ) saxs_options.saxs_iq_crysol = parameters[ "saxs_options.saxs_iq_crysol" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_debye" ) ) saxs_options.sans_iq_native_debye = parameters[ "saxs_options.sans_iq_native_debye" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid" ) ) saxs_options.sans_iq_native_hybrid = parameters[ "saxs_options.sans_iq_native_hybrid" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid2" ) ) saxs_options.sans_iq_native_hybrid2 = parameters[ "saxs_options.sans_iq_native_hybrid2" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid3" ) ) saxs_options.sans_iq_native_hybrid3 = parameters[ "saxs_options.sans_iq_native_hybrid3" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_fast" ) ) saxs_options.sans_iq_native_fast = parameters[ "saxs_options.sans_iq_native_fast" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_fast_compute_pr" ) ) saxs_options.sans_iq_native_fast_compute_pr = parameters[ "saxs_options.sans_iq_native_fast_compute_pr" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_cryson" ) ) saxs_options.sans_iq_cryson = parameters[ "saxs_options.sans_iq_cryson" ] == "1";
   if ( parameters.count( "saxs_options.hybrid2_q_points" ) ) saxs_options.hybrid2_q_points = parameters[ "saxs_options.hybrid2_q_points" ].toUInt();
   if ( parameters.count( "saxs_options.iq_ask" ) ) saxs_options.iq_ask = parameters[ "saxs_options.iq_ask" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_ask" ) ) saxs_options.iq_scale_ask = parameters[ "saxs_options.iq_scale_ask" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_angstrom" ) ) saxs_options.iq_scale_angstrom = parameters[ "saxs_options.iq_scale_angstrom" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_nm" ) ) saxs_options.iq_scale_nm = parameters[ "saxs_options.iq_scale_nm" ] == "1";
   if ( parameters.count( "saxs_options.sh_max_harmonics" ) ) saxs_options.sh_max_harmonics = parameters[ "saxs_options.sh_max_harmonics" ].toDouble();
   if ( parameters.count( "saxs_options.sh_fibonacci_grid_order" ) ) saxs_options.sh_fibonacci_grid_order = parameters[ "saxs_options.sh_fibonacci_grid_order" ].toDouble();
   if ( parameters.count( "saxs_options.crysol_hydration_shell_contrast" ) ) saxs_options.crysol_hydration_shell_contrast = parameters[ "saxs_options.crysol_hydration_shell_contrast" ].toFloat();
   if ( parameters.count( "saxs_options.crysol_default_load_difference_intensity" ) ) saxs_options.crysol_default_load_difference_intensity = parameters[ "saxs_options.crysol_default_load_difference_intensity" ] == "1";
   if ( parameters.count( "saxs_options.crysol_version_26" ) ) saxs_options.crysol_version_26 = parameters[ "saxs_options.crysol_version_26" ] == "1";
   if ( parameters.count( "saxs_options.crysol_version_3" ) ) saxs_options.crysol_version_3 = parameters[ "saxs_options.crysol_version_3" ] == "1";
   if ( parameters.count( "saxs_options.crysol_water_dummy_beads" ) ) saxs_options.crysol_water_dummy_beads = parameters[ "saxs_options.crysol_water_dummy_beads" ] == "1";
   if ( parameters.count( "saxs_options.fast_bin_size" ) ) saxs_options.fast_bin_size = parameters[ "saxs_options.fast_bin_size" ].toFloat();
   if ( parameters.count( "saxs_options.fast_modulation" ) ) saxs_options.fast_modulation = parameters[ "saxs_options.fast_modulation" ].toFloat();
   if ( parameters.count( "saxs_options.compute_saxs_coeff_for_bead_models" ) ) saxs_options.compute_saxs_coeff_for_bead_models = parameters[ "saxs_options.compute_saxs_coeff_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.compute_sans_coeff_for_bead_models" ) ) saxs_options.compute_sans_coeff_for_bead_models = parameters[ "saxs_options.compute_sans_coeff_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.default_atom_filename" ) ) saxs_options.default_atom_filename = parameters[ "saxs_options.default_atom_filename" ];
   if ( parameters.count( "saxs_options.default_hybrid_filename" ) ) saxs_options.default_hybrid_filename = parameters[ "saxs_options.default_hybrid_filename" ];
   if ( parameters.count( "saxs_options.default_saxs_filename" ) ) saxs_options.default_saxs_filename = parameters[ "saxs_options.default_saxs_filename" ];
   if ( parameters.count( "saxs_options.default_rotamer_filename" ) ) saxs_options.default_rotamer_filename = parameters[ "saxs_options.default_rotamer_filename" ];
   if ( parameters.count( "saxs_options.steric_clash_distance" ) ) saxs_options.steric_clash_distance = parameters[ "saxs_options.steric_clash_distance" ].toDouble();
   if ( parameters.count( "saxs_options.steric_clash_recheck_distance" ) ) saxs_options.steric_clash_recheck_distance = parameters[ "saxs_options.steric_clash_recheck_distance" ].toDouble();
   if ( parameters.count( "saxs_options.disable_iq_scaling" ) ) saxs_options.disable_iq_scaling = parameters[ "saxs_options.disable_iq_scaling" ] == "1";
   if ( parameters.count( "saxs_options.disable_nnls_scaling" ) ) saxs_options.disable_nnls_scaling = parameters[ "saxs_options.disable_nnls_scaling" ] == "1";
   if ( parameters.count( "saxs_options.autocorrelate" ) ) saxs_options.autocorrelate = parameters[ "saxs_options.autocorrelate" ] == "1";
   if ( parameters.count( "saxs_options.hybrid_radius_excl_vol" ) ) saxs_options.hybrid_radius_excl_vol = parameters[ "saxs_options.hybrid_radius_excl_vol" ] == "1";
   if ( parameters.count( "saxs_options.scale_excl_vol" ) ) saxs_options.scale_excl_vol = parameters[ "saxs_options.scale_excl_vol" ].toFloat();
   if ( parameters.count( "saxs_options.subtract_radius" ) ) saxs_options.subtract_radius = parameters[ "saxs_options.subtract_radius" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_minq" ) ) saxs_options.iqq_scale_minq = parameters[ "saxs_options.iqq_scale_minq" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_scale_maxq" ) ) saxs_options.iqq_scale_maxq = parameters[ "saxs_options.iqq_scale_maxq" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_scale_nnls" ) ) saxs_options.iqq_scale_nnls = parameters[ "saxs_options.iqq_scale_nnls" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_linear_offset" ) ) saxs_options.iqq_scale_linear_offset = parameters[ "saxs_options.iqq_scale_linear_offset" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_chi2_fitting" ) ) saxs_options.iqq_scale_chi2_fitting = parameters[ "saxs_options.iqq_scale_chi2_fitting" ] == "1";
   if ( parameters.count( "saxs_options.iqq_expt_data_contains_variances" ) ) saxs_options.iqq_expt_data_contains_variances = parameters[ "saxs_options.iqq_expt_data_contains_variances" ] == "1";
   if ( parameters.count( "saxs_options.iqq_ask_target_grid" ) ) saxs_options.iqq_ask_target_grid = parameters[ "saxs_options.iqq_ask_target_grid" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_play" ) ) saxs_options.iqq_scale_play = parameters[ "saxs_options.iqq_scale_play" ] == "1";
   if ( parameters.count( "saxs_options.swh_excl_vol" ) ) saxs_options.swh_excl_vol = parameters[ "saxs_options.swh_excl_vol" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_default_scaling_target" ) ) saxs_options.iqq_default_scaling_target = parameters[ "saxs_options.iqq_default_scaling_target" ];
   if ( parameters.count( "saxs_options.saxs_iq_hybrid_adaptive" ) ) saxs_options.saxs_iq_hybrid_adaptive = parameters[ "saxs_options.saxs_iq_hybrid_adaptive" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_hybrid_adaptive" ) ) saxs_options.sans_iq_hybrid_adaptive = parameters[ "saxs_options.sans_iq_hybrid_adaptive" ] == "1";
   if ( parameters.count( "saxs_options.bead_model_rayleigh" ) ) saxs_options.bead_model_rayleigh = parameters[ "saxs_options.bead_model_rayleigh" ] == "1";
   if ( parameters.count( "saxs_options.iqq_log_fitting" ) ) saxs_options.iqq_log_fitting = parameters[ "saxs_options.iqq_log_fitting" ] == "1";
   if ( parameters.count( "saxs_options.iqq_kratky_fit" ) ) saxs_options.iqq_kratky_fit = parameters[ "saxs_options.iqq_kratky_fit" ] == "1";
   if ( parameters.count( "saxs_options.iqq_use_atomic_ff" ) ) saxs_options.iqq_use_atomic_ff = parameters[ "saxs_options.iqq_use_atomic_ff" ] == "1";
   if ( parameters.count( "saxs_options.iqq_use_saxs_excl_vol" ) ) saxs_options.iqq_use_saxs_excl_vol = parameters[ "saxs_options.iqq_use_saxs_excl_vol" ] == "1";
   if ( parameters.count( "saxs_options.alt_hydration" ) ) saxs_options.alt_hydration = parameters[ "saxs_options.alt_hydration" ] == "1";
   if ( parameters.count( "saxs_options.xsr_symmop" ) ) saxs_options.xsr_symmop = parameters[ "saxs_options.xsr_symmop" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_nx" ) ) saxs_options.xsr_nx = parameters[ "saxs_options.xsr_nx" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_ny" ) ) saxs_options.xsr_ny = parameters[ "saxs_options.xsr_ny" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_griddistance" ) ) saxs_options.xsr_griddistance = parameters[ "saxs_options.xsr_griddistance" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_ncomponents" ) ) saxs_options.xsr_ncomponents = parameters[ "saxs_options.xsr_ncomponents" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_compactness_weight" ) ) saxs_options.xsr_compactness_weight = parameters[ "saxs_options.xsr_compactness_weight" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_looseness_weight" ) ) saxs_options.xsr_looseness_weight = parameters[ "saxs_options.xsr_looseness_weight" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_temperature" ) ) saxs_options.xsr_temperature = parameters[ "saxs_options.xsr_temperature" ].toDouble();
   if ( parameters.count( "hydro.zeno_zeno" ) ) hydro.zeno_zeno = parameters[ "hydro.zeno_zeno" ] == "1";
   if ( parameters.count( "hydro.zeno_interior" ) ) hydro.zeno_interior = parameters[ "hydro.zeno_interior" ] == "1";
   if ( parameters.count( "hydro.zeno_surface" ) ) hydro.zeno_surface = parameters[ "hydro.zeno_surface" ] == "1";
   if ( parameters.count( "hydro.zeno_zeno_steps" ) ) hydro.zeno_zeno_steps = parameters[ "hydro.zeno_zeno_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_interior_steps" ) ) hydro.zeno_interior_steps = parameters[ "hydro.zeno_interior_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_surface_steps" ) ) hydro.zeno_surface_steps = parameters[ "hydro.zeno_surface_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_surface_thickness" ) ) hydro.zeno_surface_thickness = parameters[ "hydro.zeno_surface_thickness" ].toFloat();
   if ( parameters.count( "hydro.zeno_surface_thickness_from_rg" ) ) hydro.zeno_surface_thickness_from_rg = parameters[ "hydro.zeno_surface_thickness_from_rg" ] == "1";
   if ( parameters.count( "hydro.zeno_surface_thickness_from_rg_a" ) ) hydro.zeno_surface_thickness_from_rg_a = parameters[ "hydro.zeno_surface_thickness_from_rg_a" ].toDouble();
   if ( parameters.count( "hydro.zeno_surface_thickness_from_rg_b" ) ) hydro.zeno_surface_thickness_from_rg_b = parameters[ "hydro.zeno_surface_thickness_from_rg_b" ].toDouble();
   if ( parameters.count( "hydro.zeno_surface_thickness_from_rg_c" ) ) hydro.zeno_surface_thickness_from_rg_c = parameters[ "hydro.zeno_surface_thickness_from_rg_c" ].toDouble();
   if ( parameters.count( "misc.hydro_supc" ) ) misc.hydro_supc = parameters[ "misc.hydro_supc" ] == "1";
   if ( parameters.count( "misc.hydro_zeno" ) ) misc.hydro_zeno = parameters[ "misc.hydro_zeno" ] == "1";
   if ( parameters.count( "batch.saxs_search" ) ) batch.saxs_search = parameters[ "batch.saxs_search" ] == "1";
   if ( parameters.count( "batch.zeno" ) ) batch.zeno = parameters[ "batch.zeno" ] == "1";

   if ( parameters.count( "saxs_options.ignore_errors" ) ) saxs_options.ignore_errors = parameters[ "saxs_options.ignore_errors" ] == "1";
   if ( parameters.count( "saxs_options.trunc_pr_dmax_target" ) ) saxs_options.trunc_pr_dmax_target = parameters[ "saxs_options.trunc_pr_dmax_target" ] == "1";
   if ( parameters.count( "saxs_options.alt_ff" ) ) saxs_options.alt_ff = parameters[ "saxs_options.alt_ff" ] == "1";
   if ( parameters.count( "saxs_options.crysol_explicit_hydrogens" ) ) saxs_options.crysol_explicit_hydrogens = parameters[ "saxs_options.crysol_explicit_hydrogens" ] == "1";
   if ( parameters.count( "saxs_options.use_somo_ff" ) ) saxs_options.use_somo_ff = parameters[ "saxs_options.use_somo_ff" ] == "1";
   if ( parameters.count( "saxs_options.five_term_gaussians" ) ) saxs_options.five_term_gaussians = parameters[ "saxs_options.five_term_gaussians" ] == "1";
   if ( parameters.count( "saxs_options.iq_exact_q" ) ) saxs_options.iq_exact_q = parameters[ "saxs_options.iq_exact_q" ] == "1";
   if ( parameters.count( "saxs_options.use_iq_target_ev" ) ) saxs_options.use_iq_target_ev = parameters[ "saxs_options.use_iq_target_ev" ] == "1";
   if ( parameters.count( "saxs_options.set_iq_target_ev_from_vbar" ) ) saxs_options.set_iq_target_ev_from_vbar = parameters[ "saxs_options.set_iq_target_ev_from_vbar" ] == "1";
   if ( parameters.count( "saxs_options.iq_target_ev" ) ) saxs_options.iq_target_ev = parameters[ "saxs_options.iq_target_ev" ].toDouble();
   if ( parameters.count( "saxs_options.hydration_rev_asa" ) ) saxs_options.hydration_rev_asa = parameters[ "saxs_options.hydration_rev_asa" ] == "1";
   if ( parameters.count( "saxs_options.compute_exponentials" ) ) saxs_options.compute_exponentials = parameters[ "saxs_options.compute_exponentials" ] == "1";
   if ( parameters.count( "saxs_options.compute_exponential_terms" ) ) saxs_options.compute_exponential_terms = parameters[ "saxs_options.compute_exponential_terms" ].toUInt();
   if ( parameters.count( "saxs_options.dummy_saxs_name" ) ) saxs_options.dummy_saxs_name = parameters[ "saxs_options.dummy_saxs_name" ];
   if ( parameters.count( "saxs_options.multiply_iq_by_atomic_volume" ) ) saxs_options.multiply_iq_by_atomic_volume = parameters[ "saxs_options.multiply_iq_by_atomic_volume" ] == "1";
   if ( parameters.count( "saxs_options.dummy_atom_pdbs_in_nm" ) ) saxs_options.dummy_atom_pdbs_in_nm = parameters[ "saxs_options.dummy_atom_pdbs_in_nm" ] == "1";
   if ( parameters.count( "saxs_options.iq_global_avg_for_bead_models" ) ) saxs_options.iq_global_avg_for_bead_models = parameters[ "saxs_options.iq_global_avg_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ) ) saxs_options.apply_loaded_sf_repeatedly_to_pdb = parameters[ "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_var_len_sf" ) ) saxs_options.bead_models_use_var_len_sf = parameters[ "saxs_options.bead_models_use_var_len_sf" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_var_len_sf_max" ) ) saxs_options.bead_models_var_len_sf_max = parameters[ "saxs_options.bead_models_var_len_sf_max" ].toUInt();
   if ( parameters.count( "saxs_options.bead_models_use_gsm_fitting" ) ) saxs_options.bead_models_use_gsm_fitting = parameters[ "saxs_options.bead_models_use_gsm_fitting" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_quick_fitting" ) ) saxs_options.bead_models_use_quick_fitting = parameters[ "saxs_options.bead_models_use_quick_fitting" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_bead_radius_ev" ) ) saxs_options.bead_models_use_bead_radius_ev = parameters[ "saxs_options.bead_models_use_bead_radius_ev" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_rho0_in_scat_factors" ) ) saxs_options.bead_models_rho0_in_scat_factors = parameters[ "saxs_options.bead_models_rho0_in_scat_factors" ] == "1";
   if ( parameters.count( "saxs_options.smooth" ) ) saxs_options.smooth = parameters[ "saxs_options.smooth" ].toUInt();
   if ( parameters.count( "saxs_options.ev_exp_mult" ) ) saxs_options.ev_exp_mult = parameters[ "saxs_options.ev_exp_mult" ].toDouble();
   if ( parameters.count( "saxs_options.sastbx_method" ) ) saxs_options.sastbx_method = parameters[ "saxs_options.sastbx_method" ].toUInt();
   if ( parameters.count( "saxs_options.saxs_iq_sastbx" ) ) saxs_options.saxs_iq_sastbx = parameters[ "saxs_options.saxs_iq_sastbx" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_sh" ) ) saxs_options.saxs_iq_native_sh = parameters[ "saxs_options.saxs_iq_native_sh" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_sh" ) ) saxs_options.sans_iq_native_sh = parameters[ "saxs_options.sans_iq_native_sh" ] == "1";
   if ( parameters.count( "saxs_options.alt_sh1" ) ) saxs_options.alt_sh1 = parameters[ "saxs_options.alt_sh1" ] == "1";
   if ( parameters.count( "saxs_options.alt_sh2" ) ) saxs_options.alt_sh2 = parameters[ "saxs_options.alt_sh2" ] == "1";
   if ( parameters.count( "grid.create_nmr_bead_pdb" ) ) grid.create_nmr_bead_pdb = parameters[ "grid.create_nmr_bead_pdb" ] == "1";
   if ( parameters.count( "batch.compute_iq_only_avg" ) ) batch.compute_iq_only_avg = parameters[ "batch.compute_iq_only_avg" ] == "1";
   if ( parameters.count( "asa.vvv" ) ) asa.vvv = parameters[ "asa.vvv" ] == "1";
   if ( parameters.count( "asa.vvv_probe_radius" ) ) asa.vvv_probe_radius = parameters[ "asa.vvv_probe_radius" ].toFloat();
   if ( parameters.count( "asa.vvv_grid_dR" ) ) asa.vvv_grid_dR = parameters[ "asa.vvv_grid_dR" ].toFloat();
   if ( parameters.count( "misc.export_msroll" ) ) misc.export_msroll = parameters[ "misc.export_msroll" ] == "1";
   if ( parameters.count( "misc.parallel_grpy" ) ) misc.parallel_grpy = parameters[ "misc.parallel_grpy" ] == "1";
   if ( parameters.count( "misc.auto_calc_hydro_method" ) ) misc.auto_calc_hydro_method = (CALC_HYDRO_METHOD) parameters[ "misc.auto_calc_hydro_method" ].toInt();

   if ( parameters.count( "saxs_options.qstart" ) ) saxs_options.qstart = parameters[ "saxs_options.qstart" ].toDouble();
   if ( parameters.count( "saxs_options.qend" ) ) saxs_options.qend = parameters[ "saxs_options.qend" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_csv" ) ) saxs_options.guinier_csv = parameters[ "saxs_options.guinier_csv" ] == "1";
   if ( parameters.count( "saxs_options.guinier_csv_filename" ) ) saxs_options.guinier_csv_filename = parameters[ "saxs_options.guinier_csv_filename" ];
   if ( parameters.count( "saxs_options.qRgmax" ) ) saxs_options.qRgmax = parameters[ "saxs_options.qRgmax" ].toDouble();
   if ( parameters.count( "saxs_options.pointsmin" ) ) saxs_options.pointsmin = parameters[ "saxs_options.pointsmin" ].toUInt();
   if ( parameters.count( "saxs_options.pointsmax" ) ) saxs_options.pointsmax = parameters[ "saxs_options.pointsmax" ].toUInt();

   if ( parameters.count( "saxs_options.cs_qRgmax" ) ) saxs_options.cs_qRgmax = parameters[ "saxs_options.cs_qRgmax" ].toDouble();
   if ( parameters.count( "saxs_options.cs_qstart" ) ) saxs_options.cs_qstart = parameters[ "saxs_options.cs_qstart" ].toDouble();
   if ( parameters.count( "saxs_options.cs_qend" ) ) saxs_options.cs_qend = parameters[ "saxs_options.cs_qend" ].toDouble();
   if ( parameters.count( "saxs_options.conc" ) ) saxs_options.conc = parameters[ "saxs_options.conc" ].toDouble();
   if ( parameters.count( "saxs_options.psv" ) ) saxs_options.psv = parameters[ "saxs_options.psv" ].toDouble();
   if ( parameters.count( "saxs_options.use_cs_psv" ) ) saxs_options.use_cs_psv = parameters[ "saxs_options.use_cs_psv" ] == "1";
   if ( parameters.count( "saxs_options.cs_psv" ) ) saxs_options.cs_psv = parameters[ "saxs_options.cs_psv" ].toDouble();
   if ( parameters.count( "saxs_options.I0_exp" ) ) saxs_options.I0_exp = parameters[ "saxs_options.I0_exp" ].toDouble();
   if ( parameters.count( "saxs_options.I0_theo" ) ) saxs_options.I0_theo = parameters[ "saxs_options.I0_theo" ].toDouble();
   if ( parameters.count( "saxs_options.diffusion_len" ) ) saxs_options.diffusion_len = parameters[ "saxs_options.diffusion_len" ].toDouble();
   if ( parameters.count( "saxs_options.nucleon_mass" ) ) saxs_options.nucleon_mass = parameters[ "saxs_options.nucleon_mass" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_outlier_reject" ) ) saxs_options.guinier_outlier_reject = parameters[ "saxs_options.guinier_outlier_reject" ] == "1";
   if ( parameters.count( "saxs_options.guinier_outlier_reject_dist" ) ) saxs_options.guinier_outlier_reject_dist = parameters[ "saxs_options.guinier_outlier_reject_dist" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_use_sd" ) ) saxs_options.guinier_use_sd = parameters[ "saxs_options.guinier_use_sd" ] == "1";
   if ( parameters.count( "saxs_options.guinier_use_standards" ) ) saxs_options.guinier_use_standards = parameters[ "saxs_options.guinier_use_standards" ] == "1";

   if ( parameters.count( "saxs_options.cryson_sh_max_harmonics" ) ) saxs_options.cryson_sh_max_harmonics = parameters[ "saxs_options.cryson_sh_max_harmonics" ].toUInt();
   if ( parameters.count( "saxs_options.cryson_sh_fibonacci_grid_order" ) ) saxs_options.cryson_sh_fibonacci_grid_order = parameters[ "saxs_options.cryson_sh_fibonacci_grid_order" ].toUInt();
   if ( parameters.count( "saxs_options.cryson_hydration_shell_contrast" ) ) saxs_options.cryson_hydration_shell_contrast = parameters[ "saxs_options.cryson_hydration_shell_contrast" ].toFloat();
   if ( parameters.count( "saxs_options.cryson_manual_hs" ) ) saxs_options.cryson_manual_hs = parameters[ "saxs_options.cryson_manual_hs" ] == "1";

   if ( parameters.count( "advanced_config.temp_dir_threshold_mb" ) ) advanced_config.temp_dir_threshold_mb = parameters[ "advanced_config.temp_dir_threshold_mb" ].toInt();

   // vectors to read:

   saxs_options.dummy_saxs_names.clear( );
   if ( parameters.count( "saxs_options.dummy_saxs_names" ) )
   {
      QStringList qsl_tmp = (parameters[ "saxs_options.dummy_saxs_names" ] ).split( "\n" , Qt::SkipEmptyParts );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         saxs_options.dummy_saxs_names.push_back( qsl_tmp[ i ] );
      }
   }

   batch.file.clear( );
   if ( parameters.count( "batch.file" ) )
   {
      QStringList qsl_tmp = (parameters[ "batch.file" ] ).split( "\n" , Qt::SkipEmptyParts );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         batch.file.push_back( qsl_tmp[ i ] );
      }
   }

   save_params.field.clear( );
   if ( parameters.count( "save_params.field" ) )
   {
      QStringList qsl_tmp = (parameters[ "save_params.field" ] ).split( "\n" , Qt::SkipEmptyParts );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         save_params.field.push_back( qsl_tmp[ i ] );
      }
   }

   {
      gparams.clear( );
      QRegExp rx_gparam( "^gparam:(.*)$" );
      
      for ( map < QString, QString >::iterator it = parameters.begin();
            it != parameters.end();
            it++ )
      {
         if ( rx_gparam.indexIn( it->first ) != -1 )
         {
            gparams[ rx_gparam.cap( 1 ) ] = it->second;
         }
      }
   }

   if ( parameters.count( "directory_history" ) &&
        parameters.count( "directory_last_access" ) )
   {
      directory_history      .clear( );
      directory_last_access  .clear( );
      directory_last_filetype.clear( );
      QStringList qsl_tmp1 = (parameters[ "directory_history"     ] ).split( "\n" , Qt::SkipEmptyParts );
      QStringList qsl_tmp2 = (parameters[ "directory_last_access" ] ).split( "\n" , Qt::SkipEmptyParts );
      QStringList qsl_tmp3;
      if ( parameters.count( "directory_last_filetype" ) )
      {
         qsl_tmp3 = (parameters[ "directory_last_filetype" ] ).split( "\n" , Qt::SkipEmptyParts );
         if ( qsl_tmp3.size() != qsl_tmp2.size() )
         {
            // us_qdebug( QString( "tmp3 cleared %1 %2" ).arg( qsl_tmp3.size() ).arg( qsl_tmp2.size() ) );
            qsl_tmp3.clear( );
         }
      }

      if ( qsl_tmp1.size() == qsl_tmp2.size() )
      {
         QDateTime dt;
         for ( int i = 0; i < (int) qsl_tmp1.size(); ++i )
         {
            directory_history.push_back( qsl_tmp1[ i ] );
            dt.setTime_t( qsl_tmp2[ i ].toUInt() );
            directory_last_access[ directory_history.back() ] = dt;
            if ( qsl_tmp3.size() )
            {
               // us_qdebug( QString( "adding to history tmp3 %1" ).arg( qsl_tmp3[ i ] ) );
               directory_last_filetype[ directory_history.back() ] = qsl_tmp3[ i ].replace( "____", "" );
            }
         }
      }
   }
      
   // fix up consistancy parameters

   saxs_options.cs_qstart                          = saxs_options.qstart * saxs_options.qstart;
   saxs_options.cs_qend                            = saxs_options.qend   * saxs_options.qend;

   if ( !saxs_options.cryson_manual_hs )
   {
      saxs_options.cryson_hydration_shell_contrast    = 
         saxs_options.d2o_conc * saxs_options.d2o_scat_len_dens +
         ( 1e0 - saxs_options.d2o_conc ) * ( saxs_options.h2o_scat_len_dens );
   }

   if ( !gparams.count( "guinier_qRtmax" ) ||
        gparams[ "guinier_qRtmax" ].toDouble() == 0e0 )
   {
      gparams[ "guinier_qRtmax" ]                     = "1";
   }

   if ( !gparams.count( "guinier_electron_nucleon_ratio" ) ||
        gparams[ "guinier_electron_nucleon_ratio" ].toDouble() == 0e0 )
   {
      gparams[ "guinier_electron_nucleon_ratio" ]     = "1.87e0";
   }

   save_params_force_results_name( save_params );

   return true;
}

#if defined( OLD_WAY )
{
   ts << "SOMO Config file - computer generated, please do not edit...\n";

   ts << replicate_o_r_method_somo << "\t\t# Replicate overlap removal method flag\n";
      
   ts << sidechain_overlap.remove_overlap << "\t\t# Remove side chain overlaps flag\n";
   ts << sidechain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << sidechain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << sidechain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << sidechain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << sidechain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << sidechain_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
   ts << sidechain_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << sidechain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << mainchain_overlap.remove_overlap << "\t\t# Remove mainchain overlaps flag\n";
   ts << mainchain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << mainchain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << mainchain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << mainchain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << mainchain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << mainchain_overlap.remove_sync_percent << "\t\t# percent synchronously step\n";
   ts << mainchain_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << mainchain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << buried_overlap.remove_overlap << "\t\t# Remove buried beads overlaps flag\n";
   ts << buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
   ts << buried_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << replicate_o_r_method_grid << "\t\t# Replicate overlap removal method flag\n";

   ts << grid_exposed_overlap.remove_overlap << "\t\t# Remove exposed grid bead overlaps flag\n";
   ts << grid_exposed_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << grid_exposed_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << grid_exposed_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << grid_exposed_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << grid_exposed_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << grid_exposed_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
   ts << grid_exposed_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << grid_exposed_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << grid_buried_overlap.remove_overlap << "\t\t# Remove buried grid bead overlaps flag\n";
   ts << grid_buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << grid_buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << grid_buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << grid_buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << grid_buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << grid_buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
   ts << grid_buried_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << grid_buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << grid_overlap.remove_overlap << "\t\t# Remove grid bead overlaps flag\n";
   ts << grid_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
   ts << grid_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
   ts << grid_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
   ts << grid_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
   ts << grid_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
   ts << grid_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
   ts << grid_overlap.translate_out << "\t\t# Outward translation flag\n";
   ts << grid_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

   ts << overlap_tolerance << "\t\t# bead overlap tolerance\n";

   ts << bead_output.output << "\t\t# flag for selecting output format\n";
   ts << bead_output.sequence << "\t\t# flag for selecting sequence format\n";
   ts << bead_output.correspondence << "\t\t# flag for residue correspondence (BEAMS only)\n";
   ts << asa.probe_radius << "\t\t# probe radius in angstrom\n";
   ts << asa.probe_recheck_radius << "\t\t# probe recheck radius in angstrom\n";
   ts << asa.threshold << "\t\t# SOMO ASA threshold\n";
   ts << asa.threshold_percent << "\t\t# SOMO ASA threshold percent\n";
   ts << asa.grid_threshold << "\t\t# Grid ASA threshold\n";
   ts << asa.grid_threshold_percent << "\t\t# Grid ASA threshold percent\n";
   ts << asa.calculation << "\t\t# flag for calculation of ASA\n";
   ts << asa.recheck_beads << "\t\t# flag for rechecking beads\n";
   ts << asa.method << "\t\t# flag for ASAB1/Surfracer method\n";
   ts << asa.asab1_step << "\t\t# ASAB1 step size\n";

   ts << grid.cubic << "\t\t# flag to apply cubic grid\n";
   ts << grid.hydrate << "\t\t# flag to hydrate original model (grid)\n";
   ts << grid.center << "\t\t# flag for positioning bead in center of mass or cubelet (grid)\n";
   ts << grid.tangency << "\t\t# flag for expanding beads to tangency (grid)\n";
   ts << grid.cube_side << "\t\t# Length of cube side (grid)\n";
   ts << grid.enable_asa << "\t\t# flag for enabling asa options (grid)\n";

   ts << misc.hydrovol << "\t\t# hydration volume\n";
   ts << misc.compute_vbar << "\t\t# flag for selecting vbar calculation\n";
   ts << misc.vbar << "\t\t# vbar value\n";
   ts << misc.vbar_temperature << "\t\t# manual vbar temperature \n";
   ts << misc.pb_rule_on << "\t\t# flag for usage of peptide bond rule\n";
   ts << misc.avg_radius << "\t\t# Average atomic radius value\n";
   ts << misc.avg_mass << "\t\t# Average atomic mass value\n";
   ts << misc.avg_hydration << "\t\t# Average atomic hydration value\n";
   ts << misc.avg_volume << "\t\t# Average bead/atom volume value\n";
   ts << misc.avg_vbar << "\t\t# Average vbar value\n";

   ts << hydro.unit << "\t\t# exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)\n";
   ts << hydro.solvent_name << "\t\t# solvent name\n";
   ts << hydro.solvent_acronym << "\t\t# solvent acronym\n";
   ts << hydro.temperature << "\t\t# solvent temperature in degrees C\n";
   ts << hydro.solvent_viscosity << "\t\t# viscosity of the solvent in cP\n";
   ts << hydro.solvent_density << "\t\t# denisty of the solvent (g/ml)\n";
   ts << hydro.reference_system << "\t\t# flag for reference system\n";
   ts << hydro.boundary_cond << "\t\t# flag for boundary condition: false: stick, true: slip\n";
   ts << hydro.volume_correction << "\t\t# flag for volume correction - false: Automatic, true: manual\n";
   ts << hydro.volume << "\t\t# volume correction value for manual entry\n";
   ts << hydro.mass_correction << "\t\t# flag for mass correction: false: Automatic, true: manual\n";
   ts << hydro.mass << "\t\t# mass correction value for manual entry\n";
   ts << hydro.bead_inclusion << "\t\t# flag for bead inclusion in computation - false: exclude hidden beads; true: use all beads\n";
   ts << hydro.rotational << "\t\t# flag false: include beads in volume correction for rotational diffusion, true: exclude\n";
   ts << hydro.viscosity << "\t\t# flag false: include beads in volume correction for intrinsic viscosity, true: exclude\n";
   ts << hydro.overlap_cutoff << "\t\t# flag for overlap cutoff: false: same as in model building, true: enter manually\n";
   ts << hydro.overlap << "\t\t# overlap value\n";

   ts << pdb_vis.visualization << "\t\t# PDB visualization option\n";
   ts << pdb_vis.filename << Qt::endl; // "\t\t# RasMol color filename\n";

   ts << pdb_parse.skip_hydrogen << "\t\t# skip hydrogen atoms?\n";
   ts << pdb_parse.skip_water << "\t\t# skip water molecules?\n";
   ts << pdb_parse.alternate << "\t\t# skip alternate conformations?\n";
   ts << pdb_parse.find_sh << "\t\t# find SH groups?\n";
   ts << pdb_parse.missing_residues << "\t\t# how to handle missing residues\n";
   ts << pdb_parse.missing_atoms << "\t\t# how to handle missing atoms\n";

   ts << saxs_options.water_e_density << "\t\t# Water electron density\n";

   ts << saxs_options.h_scat_len << "\t\t# H scattering length (*10^-12 cm)\n";
   ts << saxs_options.d_scat_len << "\t\t# D scattering length (*10^-12 cm)\n";
   ts << saxs_options.h2o_scat_len_dens << "\t\t# H2O scattering length density (*10^-10 cm^2)\n";
   ts << saxs_options.d2o_scat_len_dens << "\t\t# D2O scattering length density (*10^-10 cm^2)\n";
   ts << saxs_options.d2o_conc << "\t\t# D2O concentration (0 to 1)\n";
   ts << saxs_options.frac_of_exch_pep << "\t\t# Fraction of exchanged peptide H (0 to 1)\n";

   ts << saxs_options.wavelength << "\t\t# scattering wavelength\n";
   ts << saxs_options.start_angle << "\t\t# starting angle\n";
   ts << saxs_options.end_angle << "\t\t# ending angle\n";
   ts << saxs_options.delta_angle << "\t\t# angle stepsize\n";
   ts << saxs_options.max_size << "\t\t# maximum size\n";
   ts << saxs_options.bin_size << "\t\t# bin size\n";
   ts << saxs_options.hydrate_pdb << "\t\t# hydrate PDB model? true = yes\n";
   ts << saxs_options.curve << "\t\t# 0 = raw, 1 = saxs, 2 = sans\n";
   ts << saxs_options.saxs_sans << "\t\t# 0 = saxs, 1 = sans\n";

   ts << bd_options.threshold_pb_pb << "\t\t# bd_options.threshold_pb_pb\n";
   ts << bd_options.threshold_pb_sc << "\t\t# bd_options.threshold_pb_sc\n";
   ts << bd_options.threshold_sc_sc << "\t\t# bd_options.threshold_sc_sc\n";
   ts << bd_options.do_rr << "\t\t# bd_options.do_rr\n";
   ts << bd_options.force_chem << "\t\t# bd_options.force_chem\n";
   ts << bd_options.bead_size_type << "\t\t# bd_options.bead_size_type\n";
   ts << bd_options.show_pdb << "\t\t# bd_options.show_pdb\n";
   ts << bd_options.run_browflex << "\t\t# bd_options.run_browflex\n";
   ts << bd_options.tprev << "\t\t# bd_options.tprev\n";
   ts << bd_options.ttraj << "\t\t# bd_options.ttraj\n";
   ts << bd_options.deltat << "\t\t# bd_options.deltat\n";
   ts << bd_options.npadif << "\t\t# bd_options.npadif\n";
   ts << bd_options.nconf << "\t\t# bd_options.nconf\n";
   ts << bd_options.inter << "\t\t# bd_options.inter\n";
   ts << bd_options.iorder << "\t\t# bd_options.iorder\n";
   ts << bd_options.iseed << "\t\t# bd_options.iseed\n";
   ts << bd_options.icdm << "\t\t# bd_options.icdm\n";
   ts << bd_options.chem_pb_pb_bond_type << "\t\t# bd_options.chem_pb_pb_bond_type\n";
   ts << bd_options.compute_chem_pb_pb_force_constant << "\t\t# bd_options.compute_chem_pb_pb_force_constant\n";
   ts << bd_options.chem_pb_pb_force_constant << "\t\t# bd_options.chem_pb_pb_force_constant\n";
   ts << bd_options.compute_chem_pb_pb_equilibrium_dist << "\t\t# bd_options.compute_chem_pb_pb_equilibrium_dist\n";
   ts << bd_options.chem_pb_pb_equilibrium_dist << "\t\t# bd_options.chem_pb_pb_equilibrium_dist\n";
   ts << bd_options.compute_chem_pb_pb_max_elong << "\t\t# bd_options.compute_chem_pb_pb_max_elong\n";
   ts << bd_options.chem_pb_pb_max_elong << "\t\t# bd_options.chem_pb_pb_max_elong\n";
   ts << bd_options.chem_pb_sc_bond_type << "\t\t# bd_options.chem_pb_sc_bond_type\n";
   ts << bd_options.compute_chem_pb_sc_force_constant << "\t\t# bd_options.compute_chem_pb_sc_force_constant\n";
   ts << bd_options.chem_pb_sc_force_constant << "\t\t# bd_options.chem_pb_sc_force_constant\n";
   ts << bd_options.compute_chem_pb_sc_equilibrium_dist << "\t\t# bd_options.compute_chem_pb_sc_equilibrium_dist\n";
   ts << bd_options.chem_pb_sc_equilibrium_dist << "\t\t# bd_options.chem_pb_sc_equilibrium_dist\n";
   ts << bd_options.compute_chem_pb_sc_max_elong << "\t\t# bd_options.compute_chem_pb_sc_max_elong\n";
   ts << bd_options.chem_pb_sc_max_elong << "\t\t# bd_options.chem_pb_sc_max_elong\n";
   ts << bd_options.chem_sc_sc_bond_type << "\t\t# bd_options.chem_sc_sc_bond_type\n";
   ts << bd_options.compute_chem_sc_sc_force_constant << "\t\t# bd_options.compute_chem_sc_sc_force_constant\n";
   ts << bd_options.chem_sc_sc_force_constant << "\t\t# bd_options.chem_sc_sc_force_constant\n";
   ts << bd_options.compute_chem_sc_sc_equilibrium_dist << "\t\t# bd_options.compute_chem_sc_sc_equilibrium_dist\n";
   ts << bd_options.chem_sc_sc_equilibrium_dist << "\t\t# bd_options.chem_sc_sc_equilibrium_dist\n";
   ts << bd_options.compute_chem_sc_sc_max_elong << "\t\t# bd_options.compute_chem_sc_sc_max_elong\n";
   ts << bd_options.chem_sc_sc_max_elong << "\t\t# bd_options.chem_sc_sc_max_elong\n";
   ts << bd_options.pb_pb_bond_type << "\t\t# bd_options.pb_pb_bond_type\n";
   ts << bd_options.compute_pb_pb_force_constant << "\t\t# bd_options.compute_pb_pb_force_constant\n";
   ts << bd_options.pb_pb_force_constant << "\t\t# bd_options.pb_pb_force_constant\n";
   ts << bd_options.compute_pb_pb_equilibrium_dist << "\t\t# bd_options.compute_pb_pb_equilibrium_dist\n";
   ts << bd_options.pb_pb_equilibrium_dist << "\t\t# bd_options.pb_pb_equilibrium_dist\n";
   ts << bd_options.compute_pb_pb_max_elong << "\t\t# bd_options.compute_pb_pb_max_elong\n";
   ts << bd_options.pb_pb_max_elong << "\t\t# bd_options.pb_pb_max_elong\n";
   ts << bd_options.pb_sc_bond_type << "\t\t# bd_options.pb_sc_bond_type\n";
   ts << bd_options.compute_pb_sc_force_constant << "\t\t# bd_options.compute_pb_sc_force_constant\n";
   ts << bd_options.pb_sc_force_constant << "\t\t# bd_options.pb_sc_force_constant\n";
   ts << bd_options.compute_pb_sc_equilibrium_dist << "\t\t# bd_options.compute_pb_sc_equilibrium_dist\n";
   ts << bd_options.pb_sc_equilibrium_dist << "\t\t# bd_options.pb_sc_equilibrium_dist\n";
   ts << bd_options.compute_pb_sc_max_elong << "\t\t# bd_options.compute_pb_sc_max_elong\n";
   ts << bd_options.pb_sc_max_elong << "\t\t# bd_options.pb_sc_max_elong\n";
   ts << bd_options.sc_sc_bond_type << "\t\t# bd_options.sc_sc_bond_type\n";
   ts << bd_options.compute_sc_sc_force_constant << "\t\t# bd_options.compute_sc_sc_force_constant\n";
   ts << bd_options.sc_sc_force_constant << "\t\t# bd_options.sc_sc_force_constant\n";
   ts << bd_options.compute_sc_sc_equilibrium_dist << "\t\t# bd_options.compute_sc_sc_equilibrium_dist\n";
   ts << bd_options.sc_sc_equilibrium_dist << "\t\t# bd_options.sc_sc_equilibrium_dist\n";
   ts << bd_options.compute_sc_sc_max_elong << "\t\t# bd_options.compute_sc_sc_max_elong\n";
   ts << bd_options.sc_sc_max_elong << "\t\t# bd_options.sc_sc_max_elong\n";
   ts << bd_options.nmol << "\t\t# bd_options.nmol\n";

   ts << anaflex_options.run_anaflex << "\t\t# anaflex_options.run_anaflex\n";
   ts << anaflex_options.nfrec << "\t\t# anaflex_options.nfrec\n";
   ts << anaflex_options.instprofiles << "\t\t# anaflex_options.instprofiles\n";
   ts << anaflex_options.run_mode_1 << "\t\t# anaflex_options.run_mode_1\n";
   ts << anaflex_options.run_mode_1_1 << "\t\t# anaflex_options.run_mode_1_1\n";
   ts << anaflex_options.run_mode_1_2 << "\t\t# anaflex_options.run_mode_1_2\n";
   ts << anaflex_options.run_mode_1_3 << "\t\t# anaflex_options.run_mode_1_3\n";
   ts << anaflex_options.run_mode_1_4 << "\t\t# anaflex_options.run_mode_1_4\n";
   ts << anaflex_options.run_mode_1_5 << "\t\t# anaflex_options.run_mode_1_5\n";
   ts << anaflex_options.run_mode_1_7 << "\t\t# anaflex_options.run_mode_1_7\n";
   ts << anaflex_options.run_mode_1_8 << "\t\t# anaflex_options.run_mode_1_8\n";
   ts << anaflex_options.run_mode_1_12 << "\t\t# anaflex_options.run_mode_1_12\n";
   ts << anaflex_options.run_mode_1_13 << "\t\t# anaflex_options.run_mode_1_13\n";
   ts << anaflex_options.run_mode_1_14 << "\t\t# anaflex_options.run_mode_1_14\n";
   ts << anaflex_options.run_mode_1_18 << "\t\t# anaflex_options.run_mode_1_18\n";
   ts << anaflex_options.run_mode_1_20 << "\t\t# anaflex_options.run_mode_1_20\n";
   ts << anaflex_options.run_mode_1_24 << "\t\t# anaflex_options.run_mode_1_24\n";
   ts << anaflex_options.run_mode_2 << "\t\t# anaflex_options.run_mode_2\n";
   ts << anaflex_options.run_mode_2_1 << "\t\t# anaflex_options.run_mode_2_1\n";
   ts << anaflex_options.run_mode_2_2 << "\t\t# anaflex_options.run_mode_2_2\n";
   ts << anaflex_options.run_mode_2_3 << "\t\t# anaflex_options.run_mode_2_3\n";
   ts << anaflex_options.run_mode_2_4 << "\t\t# anaflex_options.run_mode_2_4\n";
   ts << anaflex_options.run_mode_2_5 << "\t\t# anaflex_options.run_mode_2_5\n";
   ts << anaflex_options.run_mode_2_7 << "\t\t# anaflex_options.run_mode_2_7\n";
   ts << anaflex_options.run_mode_2_8 << "\t\t# anaflex_options.run_mode_2_8\n";
   ts << anaflex_options.run_mode_2_12 << "\t\t# anaflex_options.run_mode_2_12\n";
   ts << anaflex_options.run_mode_2_13 << "\t\t# anaflex_options.run_mode_2_13\n";
   ts << anaflex_options.run_mode_2_14 << "\t\t# anaflex_options.run_mode_2_14\n";
   ts << anaflex_options.run_mode_2_18 << "\t\t# anaflex_options.run_mode_2_18\n";
   ts << anaflex_options.run_mode_2_20 << "\t\t# anaflex_options.run_mode_2_20\n";
   ts << anaflex_options.run_mode_2_24 << "\t\t# anaflex_options.run_mode_2_24\n";
   ts << anaflex_options.run_mode_3 << "\t\t# anaflex_options.run_mode_3\n";
   ts << anaflex_options.run_mode_3_1 << "\t\t# anaflex_options.run_mode_3_1\n";
   ts << anaflex_options.run_mode_3_5 << "\t\t# anaflex_options.run_mode_3_5\n";
   ts << anaflex_options.run_mode_3_9 << "\t\t# anaflex_options.run_mode_3_9\n";
   ts << anaflex_options.run_mode_3_10 << "\t\t# anaflex_options.run_mode_3_10\n";
   ts << anaflex_options.run_mode_3_14 << "\t\t# anaflex_options.run_mode_3_14\n";
   ts << anaflex_options.run_mode_3_15 << "\t\t# anaflex_options.run_mode_3_15\n";
   ts << anaflex_options.run_mode_3_16 << "\t\t# anaflex_options.run_mode_3_16\n";
   ts << anaflex_options.run_mode_4 << "\t\t# anaflex_options.run_mode_4\n";
   ts << anaflex_options.run_mode_4_1 << "\t\t# anaflex_options.run_mode_4_1\n";
   ts << anaflex_options.run_mode_4_6 << "\t\t# anaflex_options.run_mode_4_6\n";
   ts << anaflex_options.run_mode_4_7 << "\t\t# anaflex_options.run_mode_4_7\n";
   ts << anaflex_options.run_mode_4_8 << "\t\t# anaflex_options.run_mode_4_8\n";
   ts << anaflex_options.run_mode_9 << "\t\t# anaflex_options.run_mode_9\n";
   ts << anaflex_options.ntimc << "\t\t# anaflex_options.ntimc\n";
   ts << anaflex_options.tmax << "\t\t# anaflex_options.tmax\n";
   ts << anaflex_options.run_mode_3_5_iii << "\t\t# anaflex_options.run_mode_3_5_iii\n";
   ts << anaflex_options.run_mode_3_5_jjj << "\t\t# anaflex_options.run_mode_3_5_jjj\n";
   ts << anaflex_options.run_mode_3_10_theta << "\t\t# anaflex_options.run_mode_3_10_theta\n";
   ts << anaflex_options.run_mode_3_10_refractive_index << "\t\t# anaflex_options.run_mode_3_10_refractive_index\n";
   ts << anaflex_options.run_mode_3_10_lambda << "\t\t# anaflex_options.run_mode_3_10_lambda\n";
   ts << anaflex_options.run_mode_3_14_iii << "\t\t# anaflex_options.run_mode_3_14_iii\n";
   ts << anaflex_options.run_mode_3_14_jjj << "\t\t# anaflex_options.run_mode_3_14_jjj\n";
      
   ts << batch.missing_atoms << "\t\t# batch missing atom handling\n";
   ts << batch.missing_residues << "\t\t# batch missing residue handling\n";
   ts << batch.somo << "\t\t# batch run somo\n";
   ts << batch.grid << "\t\t# batch run grid\n";
   ts << batch.hydro << "\t\t# batch run hydro\n";
   ts << batch.avg_hydro << "\t\t# batch avg hydro\n";
   ts << batch.avg_hydro_name << "\t\t# batch avg hydro name\n";
   ts << batch.height << "\t\t# batch window last height\n";
   ts << batch.width << "\t\t# batch window last width\n";
   ts << batch.file.size() << "\t\t# batch number of files to follow\n";
   for ( unsigned int i = 0; i < batch.file.size(); i++ )
   {
      ts << batch.file[i] << Qt::endl;
   }

   ts << save_params.field.size() << "\t\t# save params number of fields to follow\n";
   for ( unsigned int i = 0; i < save_params.field.size(); i++ )
   {
      ts << save_params.field[i] << Qt::endl;
   }

   ts << path_load_pdb << Qt::endl;
   ts << path_view_pdb << Qt::endl;
   ts << path_load_bead_model << Qt::endl;
   ts << path_view_asa_res << Qt::endl;
   ts << path_view_bead_model << Qt::endl;
   ts << path_open_hydro_res << Qt::endl;
   ts << saxs_options.path_load_saxs_curve << Qt::endl;
   ts << saxs_options.path_load_gnom << Qt::endl;
   ts << saxs_options.path_load_prr << Qt::endl;

   ts << asa.hydrate_probe_radius << "\t\t#asa.hydrate_probe_radius\n";
   ts << asa.hydrate_threshold << "\t\t#asa.hydrate_threshold\n";

   ts << misc.target_e_density       << "\t\t#misc.target_e_density      \n";
   ts << misc.target_volume          << "\t\t#misc.target_volume         \n";
   ts << misc.set_target_on_load_pdb << "\t\t#misc.set_target_on_load_pdb\n";
   ts << misc.equalize_radii         << "\t\t#misc.equalize_radii        \n";

   ts << dmd_options.force_chem << "\t\t#dmd_options.force_chem\n";
   ts << dmd_options.pdb_static_pairs << "\t\t#dmd_options.pdb_static_pairs\n";
   ts << dmd_options.threshold_pb_pb << "\t\t#dmd_options.threshold_pb_pb\n";
   ts << dmd_options.threshold_pb_sc << "\t\t#dmd_options.threshold_pb_sc\n";
   ts << dmd_options.threshold_sc_sc << "\t\t#dmd_options.threshold_sc_sc\n";

   ts << saxs_options.normalize_by_mw << "\t\t#saxs_options.normalize_by_mw\n";

   ts << saxs_options.saxs_iq_native_debye << "\t\t#saxs_options.saxs_iq_native_debye\n";
   ts << saxs_options.saxs_iq_native_hybrid << "\t\t#saxs_options.saxs_iq_native_hybrid\n";
   ts << saxs_options.saxs_iq_native_hybrid2 << "\t\t#saxs_options.saxs_iq_native_hybrid2\n";
   ts << saxs_options.saxs_iq_native_hybrid3 << "\t\t#saxs_options.saxs_iq_native_hybrid3\n";
   ts << saxs_options.saxs_iq_native_fast << "\t\t#saxs_options.saxs_iq_native_fast\n";
   ts << saxs_options.saxs_iq_native_fast_compute_pr << "\t\t#saxs_options.saxs_iq_native_fast_compute_pr\n";
   ts << saxs_options.saxs_iq_foxs << "\t\t#saxs_options.saxs_iq_foxs\n";
   ts << saxs_options.saxs_iq_crysol << "\t\t#saxs_options.saxs_iq_crysol\n";

   ts << saxs_options.sans_iq_native_debye << "\t\t#saxs_options.sans_iq_native_debye\n";
   ts << saxs_options.sans_iq_native_hybrid << "\t\t#saxs_options.sans_iq_native_hybrid\n";
   ts << saxs_options.sans_iq_native_hybrid2 << "\t\t#saxs_options.sans_iq_native_hybrid2\n";
   ts << saxs_options.sans_iq_native_hybrid3 << "\t\t#saxs_options.sans_iq_native_hybrid3\n";
   ts << saxs_options.sans_iq_native_fast << "\t\t#saxs_options.sans_iq_native_fast\n";
   ts << saxs_options.sans_iq_native_fast_compute_pr << "\t\t#saxs_options.sans_iq_native_fast_compute_pr\n";
   ts << saxs_options.sans_iq_cryson << "\t\t#saxs_options.sans_iq_cryson\n";

   ts << saxs_options.hybrid2_q_points << "\t\t#saxs_options.hybrid2_q_points\n";

   ts << saxs_options.iq_ask << "\t\t#saxs_options.iq_ask\n";

   ts << saxs_options.iq_scale_ask << "\t\t#saxs_options.iq_scale_ask\n";
   ts << saxs_options.iq_scale_angstrom << "\t\t#saxs_options.iq_scale_angstrom\n";
   ts << saxs_options.iq_scale_nm << "\t\t#saxs_options.iq_scale_nm\n";

   ts << saxs_options.sh_max_harmonics << "\t\t#saxs_options.sh_max_harmonics\n";
   ts << saxs_options.sh_fibonacci_grid_order << "\t\t#saxs_options.sh_fibonacci_grid_order\n";
   ts << saxs_options.crysol_hydration_shell_contrast << "\t\t#saxs_options.crysol_hydration_shell_contrast\n";
   ts << saxs_options.crysol_default_load_difference_intensity << "\t\t#saxs_options.crysol_default_load_difference_intensity\n";
   ts << saxs_options.crysol_version_26 << "\t\t#saxs_options.crysol_version_26\n";

   ts << saxs_options.fast_bin_size << "\t\t#saxs_options.fast_bin_size\n";
   ts << saxs_options.fast_modulation << "\t\t#saxs_options.fast_modulation\n";

   ts << saxs_options.compute_saxs_coeff_for_bead_models << "\t\t#saxs_options.compute_saxs_coeff_for_bead_models\n";
   ts << saxs_options.compute_sans_coeff_for_bead_models << "\t\t#saxs_options.compute_sans_coeff_for_bead_models\n";

   ts << saxs_options.default_atom_filename << Qt::endl;
   ts << saxs_options.default_hybrid_filename << Qt::endl;
   ts << saxs_options.default_saxs_filename << Qt::endl;
   ts << saxs_options.default_rotamer_filename << Qt::endl;

   ts << saxs_options.steric_clash_distance         << "\t\t#saxs_options.steric_clash_distance        \n";
   ts << saxs_options.steric_clash_recheck_distance << "\t\t#saxs_options.steric_clash_recheck_distance\n";

   ts << saxs_options.disable_iq_scaling << "\t\t#saxs_options.disable_iq_scaling\n";
   ts << saxs_options.autocorrelate << "\t\t#saxs_options.autocorrelate\n";
   ts << saxs_options.hybrid_radius_excl_vol << "\t\t#saxs_options.hybrid_radius_excl_vol\n";
   ts << saxs_options.scale_excl_vol << "\t\t#saxs_options.scale_excl_vol\n";
   ts << saxs_options.subtract_radius << "\t\t#saxs_options.subtract_radius\n";
   ts << saxs_options.iqq_scale_minq << "\t\t#saxs_options.iqq_scale_minq\n";
   ts << saxs_options.iqq_scale_maxq << "\t\t#saxs_options.iqq_scale_maxq\n";

   ts << saxs_options.iqq_scale_nnls << "\t\t#saxs_options.iqq_scale_nnls\n";
   ts << saxs_options.iqq_scale_linear_offset << "\t\t#saxs_options.iqq_scale_linear_offset\n";
   ts << saxs_options.iqq_scale_chi2_fitting << "\t\t#saxs_options.iqq_scale_chi2_fitting\n";
   ts << saxs_options.iqq_expt_data_contains_variances << "\t\t#saxs_options.iqq_expt_data_contains_variances\n";
   ts << saxs_options.iqq_ask_target_grid << "\t\t#saxs_options.iqq_ask_target_grid\n";
   ts << saxs_options.iqq_scale_play << "\t\t#saxs_options.iqq_scale_play\n";
   ts << saxs_options.swh_excl_vol << "\t\t#saxs_options.swh_excl_vol\n";
   ts << saxs_options.iqq_default_scaling_target << Qt::endl;

   ts << saxs_options.saxs_iq_hybrid_adaptive << "\t\t#saxs_options.saxs_iq_hybrid_adaptive\n";
   ts << saxs_options.sans_iq_hybrid_adaptive << "\t\t#saxs_options.sans_iq_hybrid_adaptive\n";

   ts << saxs_options.bead_model_rayleigh   << "\t\t#saxs_options.bead_model_rayleigh  \n";
   ts << saxs_options.iqq_log_fitting       << "\t\t#saxs_options.iqq_log_fitting      \n";
   ts << saxs_options.iqq_kratky_fit        << "\t\t#saxs_options.iqq_kratky_fit       \n";
   ts << saxs_options.iqq_use_atomic_ff     << "\t\t#saxs_options.iqq_use_atomic_ff    \n";
   ts << saxs_options.iqq_use_saxs_excl_vol << "\t\t#saxs_options.iqq_use_saxs_excl_vol\n";
   ts << saxs_options.alt_hydration         << "\t\t#saxs_options.alt_hydration        \n";

   ts << saxs_options.xsr_symmop                << "\t\t#saxs_options.xsr_symmop               \n";
   ts << saxs_options.xsr_nx                    << "\t\t#saxs_options.xsr_nx                   \n";
   ts << saxs_options.xsr_ny                    << "\t\t#saxs_options.xsr_ny                   \n";
   ts << saxs_options.xsr_griddistance          << "\t\t#saxs_options.xsr_griddistance         \n";
   ts << saxs_options.xsr_ncomponents           << "\t\t#saxs_options.xsr_ncomponents          \n";
   ts << saxs_options.xsr_compactness_weight    << "\t\t#saxs_options.xsr_compactness_weight   \n";
   ts << saxs_options.xsr_looseness_weight      << "\t\t#saxs_options.xsr_looseness_weight     \n";
   ts << saxs_options.xsr_temperature           << "\t\t#saxs_options.xsr_temperature          \n";

   ts << hydro.zeno_zeno              << "\t\t#hydro.zeno_zeno             \n";
   ts << hydro.zeno_interior          << "\t\t#hydro.zeno_interior         \n";
   ts << hydro.zeno_surface           << "\t\t#hydro.zeno_surface          \n";
   ts << hydro.zeno_zeno_steps        << "\t\t#hydro.zeno_zeno_steps       \n";
   ts << hydro.zeno_interior_steps    << "\t\t#hydro.zeno_interior_steps   \n";
   ts << hydro.zeno_surface_steps     << "\t\t#hydro.zeno_surface_steps    \n";
   ts << hydro.zeno_surface_thickness << "\t\t#hydro.zeno_surface_thickness\n";

   ts << misc.hydro_supc              << "\t\t#misc.hydro_supc             \n";
   ts << misc.hydro_zeno              << "\t\t#misc.hydro_zeno             \n";

   ts << batch.saxs_search << "\t\t#batch.saxs_search\n";
   ts << batch.zeno        << "\t\t#batch.zeno       \n";

   f.close();
}
#endif

void US_Hydrodyn::hard_coded_defaults()
{
   // hard coded defaults
   replicate_o_r_method_somo                                = false;

   sidechain_overlap.remove_overlap                         = true;
   sidechain_overlap.fuse_beads                             = true;
   sidechain_overlap.fuse_beads_percent                     = 70.0;
   sidechain_overlap.remove_hierarch                        = true;
   sidechain_overlap.remove_hierarch_percent                = 1.0;
   sidechain_overlap.remove_sync                            = false;
   sidechain_overlap.remove_sync_percent                    = 1.0;
   sidechain_overlap.translate_out                          = true;
   sidechain_overlap.show_translate                         = true;

   mainchain_overlap.remove_overlap                         = true;
   mainchain_overlap.fuse_beads                             = true;
   mainchain_overlap.fuse_beads_percent                     = 70.0;
   mainchain_overlap.remove_hierarch                        = true;
   mainchain_overlap.remove_hierarch_percent                = 1.0;
   mainchain_overlap.remove_sync                            = false;
   mainchain_overlap.remove_sync_percent                    = 1.0;
   mainchain_overlap.translate_out                          = false;
   mainchain_overlap.show_translate                         = false;

   buried_overlap.remove_overlap                            = true;
   buried_overlap.fuse_beads                                = false;
   buried_overlap.fuse_beads_percent                        = 0.0;
   buried_overlap.remove_hierarch                           = true;
   buried_overlap.remove_hierarch_percent                   = 1.0;
   buried_overlap.remove_sync                               = false;
   buried_overlap.remove_sync_percent                       = 1.0;
   buried_overlap.translate_out                             = false;
   buried_overlap.show_translate                            = false;

   replicate_o_r_method_grid                                = false;

   grid_exposed_overlap.remove_overlap                      = true;
   grid_exposed_overlap.fuse_beads                          = false;
   grid_exposed_overlap.fuse_beads_percent                  = 0.0;
   grid_exposed_overlap.remove_hierarch                     = false;
   grid_exposed_overlap.remove_hierarch_percent             = 1.0;
   grid_exposed_overlap.remove_sync                         = true;
   grid_exposed_overlap.remove_sync_percent                 = 1.0;
   grid_exposed_overlap.translate_out                       = true;
   grid_exposed_overlap.show_translate                      = true;

   grid_buried_overlap.remove_overlap                       = true;
   grid_buried_overlap.fuse_beads                           = false;
   grid_buried_overlap.fuse_beads_percent                   = 0.0;
   grid_buried_overlap.remove_hierarch                      = false;
   grid_buried_overlap.remove_hierarch_percent              = 1.0;
   grid_buried_overlap.remove_sync                          = true;
   grid_buried_overlap.remove_sync_percent                  = 1.0;
   grid_buried_overlap.translate_out                        = false;
   grid_buried_overlap.show_translate                       = false;

   grid_overlap.remove_overlap                              = true;
   grid_overlap.fuse_beads                                  = false;
   grid_overlap.fuse_beads_percent                          = 0.0;
   grid_overlap.remove_hierarch                             = false;
   grid_overlap.remove_hierarch_percent                     = 1.0;
   grid_overlap.remove_sync                                 = true;
   grid_overlap.remove_sync_percent                         = 1.0;
   grid_overlap.translate_out                               = false;
   grid_overlap.show_translate                              = false;

   overlap_tolerance                                        = 0.001;

   sidechain_overlap.title                                  = "exposed side chain beads";
   mainchain_overlap.title                                  = "exposed main/main and main/side chain beads";
   buried_overlap.title                                     = "buried beads";
   grid_exposed_overlap.title                               = "exposed grid beads";
   grid_buried_overlap.title                                = "buried grid beads";
   grid_overlap.title                                       = "grid beads";

   bead_output.sequence                                     = 0;
   bead_output.output                                       = 1;
   bead_output.correspondence                               = true;

   asa.probe_radius                                         = (float) 1.4;
   asa.probe_recheck_radius                                 = (float) 1.4;
   asa.threshold                                            = 20.0;
   asa.threshold_percent                                    = 50.0;
   asa.vdw_grpy_probe_radius                                = (float) 1.0;
   asa.vdw_grpy_threshold_percent                           = (float) 1.0;
   asa.grid_threshold                                       = 10.0;
   asa.grid_threshold_percent                               = 30.0;
   asa.calculation                                          = true;
   asa.recheck_beads                                        = true;
   asa.method                                               = true; // by default use ASAB1
   asa.asab1_step                                           = 1.0;

   grid.cubic                                               = true;       // apply cubic grid
   grid.hydrate                                             = true;    // true: hydrate model
   grid.center                                              = 0;    // 1: center of cubelet, 0: center of mass, 2: center of scattering
   grid.tangency                                            = false;   // true: Expand beads to tangency
   grid.cube_side                                           = 5.0;
   grid.enable_asa                                          = true;   // true: enable asa
   grid.equalize_radii_constant_volume                      = false;

   misc.hydrovol                                            = 24.041;
   misc.compute_vbar                                        = true;
   misc.vbar                                                = 0.72;
   misc.vbar_temperature                                    = 20.0;
   misc.pb_rule_on                                          = true;
   misc.avg_radius                                          = 1.68;
   misc.avg_mass                                            = 14.5;
   misc.avg_num_elect                                       = 7.7;
   misc.avg_protons                                         = 7.7;
   misc.avg_volume                                          = 15.3;
   misc.avg_vbar                                            = 0.72;
   overlap_tolerance                                        = 0.001;

   hydro.unit                                               = -10;                // exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)

   hydro.solvent_name                                       = "Water";
   hydro.solvent_acronym                                    = "w";
   hydro.temperature                                        = K20 - K0;
   hydro.pH                                                 = 7;
   hydro.solvent_viscosity                                  = VISC_20W * 100;
   hydro.solvent_density                                    = DENS_20W;
   hydro.manual_solvent_conditions                          = false;

   hydro.reference_system                                   = false;   // false: diffusion center, true: cartesian origin (default false)
   hydro.boundary_cond                                      = false;      // false: stick, true: slip (default false)
   hydro.volume_correction                                  = false;   // false: Automatic, true: manual (provide value)
   hydro.use_avg_for_volume                                 = false;   // only active if hydro.volume_correction is false
   hydro.volume                                             = 0.0;               // volume correction
   hydro.mass_correction                                    = false;      // false: Automatic, true: manual (provide value)
   hydro.mass                                               = 0.0;                  // mass correction
   hydro.bead_inclusion                                     = false;      // false: exclude hidden beads; true: use all beads
   hydro.grpy_bead_inclusion                                = false;      // false: exclude hidden beads; true: use all beads
   hydro.rotational                                         = false;         // false: include beads in volume correction for rotational diffusion, true: exclude
   hydro.viscosity                                          = false;            // false: include beads in volume correction for intrinsic viscosity, true: exclude
   hydro.overlap_cutoff                                     = false;      // false: same as in model building, true: enter manually
   hydro.overlap                                            = 0.0;               // overlap

   pdb_parse.skip_hydrogen                                  = true;
   pdb_parse.skip_water                                     = true;
   pdb_parse.alternate                                      = true;
   pdb_parse.find_sh                                        = true;
   pdb_parse.missing_residues                               = 2;
   pdb_parse.missing_atoms                                  = 2;

   saxs_options.water_e_density                             = 0.334f; // water electron density in e/A^3

   saxs_options.h_scat_len                                  = -0.3742f;        // H scattering length (*10^-12 cm)
   saxs_options.d_scat_len                                  = 0.6671f ;        // D scattering length (*10^-12 cm)
   saxs_options.h2o_scat_len_dens                           = -0.562f;  // H2O scattering length density (*10^-10 cm^2)
   saxs_options.d2o_scat_len_dens                           = 6.404f;   // D2O scattering length density (*10^-10 cm^2)
   saxs_options.d2o_conc                                    = 0.16f;             // D2O concentration (0 to 1)
   saxs_options.frac_of_exch_pep                            = 0.1f;      // Fraction of exchanged peptide H (0 to 1)

   saxs_options.wavelength                                  = 1.5;         // scattering wavelength
   saxs_options.start_angle                                 = 0.014f;     // start angle
   saxs_options.end_angle                                   = 8.214f;       // ending angle
   saxs_options.delta_angle                                 = 0.2f;       // angle stepsize
   saxs_options.max_size                                    = 40.0;          // maximum size (A)
   saxs_options.bin_size                                    = 1.0f;          // Bin size (A)
   saxs_options.hydrate_pdb                                 = false;      // Hydrate the PDB model? (true/false)
   saxs_options.curve                                       = 1;                // 0 = raw, 1 = saxs, 2 = sans
   saxs_options.saxs_sans                                   = 0;            // 0 = saxs, 1 = sans

   saxs_options.guinier_csv                                 = false;
   saxs_options.guinier_csv_filename                        = "guinier";
   saxs_options.qRgmax                                      = 1.3e0;
   saxs_options.qstart                                      = 1e-7;
   saxs_options.qend                                        = .5e0;
   saxs_options.pointsmin                                   = 10;
   saxs_options.pointsmax                                   = 100;

   bd_options.threshold_pb_pb                               = 5;
   bd_options.threshold_pb_sc                               = 5;
   bd_options.threshold_sc_sc                               = 5;
   bd_options.do_rr                                         = true;
   bd_options.force_chem                                    = true;
   bd_options.bead_size_type                                = 0;
   bd_options.show_pdb                                      = true;
   bd_options.run_browflex                                  = true;
   bd_options.tprev                                         = 8.0e-9;
   bd_options.ttraj                                         = 8.0e-6;
   bd_options.deltat                                        = 1.6e-13;
   bd_options.npadif                                        = 10;
   bd_options.nconf                                         = 1000;
   bd_options.inter                                         = 2;
   bd_options.iorder                                        = 1;
   bd_options.iseed                                         = 1234;
   bd_options.icdm                                          = 0;
   bd_options.chem_pb_pb_bond_type                          = 0;
   bd_options.compute_chem_pb_pb_force_constant             = false;
   bd_options.chem_pb_pb_force_constant                     = 10.0;
   bd_options.compute_chem_pb_pb_equilibrium_dist           = true;
   bd_options.chem_pb_pb_equilibrium_dist                   = 0.0;
   bd_options.compute_chem_pb_pb_max_elong                  = true;
   bd_options.chem_pb_pb_max_elong                          = 0.0;
   bd_options.chem_pb_sc_bond_type                          = 0;
   bd_options.compute_chem_pb_sc_force_constant             = false;
   bd_options.chem_pb_sc_force_constant                     = 10.0;
   bd_options.compute_chem_pb_sc_equilibrium_dist           = true;
   bd_options.chem_pb_sc_equilibrium_dist                   = 0.0;
   bd_options.compute_chem_pb_sc_max_elong                  = true;
   bd_options.chem_pb_sc_max_elong                          = 0.0;
   bd_options.chem_sc_sc_bond_type                          = 0;
   bd_options.compute_chem_sc_sc_force_constant             = false;
   bd_options.chem_sc_sc_force_constant                     = 10.0;
   bd_options.compute_chem_sc_sc_equilibrium_dist           = true;
   bd_options.chem_sc_sc_equilibrium_dist                   = 0.0;
   bd_options.compute_chem_sc_sc_max_elong                  = true;
   bd_options.chem_sc_sc_max_elong                          = 0.0;
   bd_options.pb_pb_bond_type                               = 0;
   bd_options.compute_pb_pb_force_constant                  = false;
   bd_options.pb_pb_force_constant                          = 10.0;
   bd_options.compute_pb_pb_equilibrium_dist                = true;
   bd_options.pb_pb_equilibrium_dist                        = 0.0;
   bd_options.compute_pb_pb_max_elong                       = true;
   bd_options.pb_pb_max_elong                               = 0.0;
   bd_options.pb_sc_bond_type                               = 0;
   bd_options.compute_pb_sc_force_constant                  = false;
   bd_options.pb_sc_force_constant                          = 10.0;
   bd_options.compute_pb_sc_equilibrium_dist                = true;
   bd_options.pb_sc_equilibrium_dist                        = 0.0;
   bd_options.compute_pb_sc_max_elong                       = true;
   bd_options.pb_sc_max_elong                               = 0.0;
   bd_options.sc_sc_bond_type                               = 0;
   bd_options.compute_sc_sc_force_constant                  = false;
   bd_options.sc_sc_force_constant                          = 10.0;
   bd_options.compute_sc_sc_equilibrium_dist                = true;
   bd_options.sc_sc_equilibrium_dist                        = 0.0;
   bd_options.compute_sc_sc_max_elong                       = true;
   bd_options.sc_sc_max_elong                               = 0.0;
   bd_options.nmol                                          = 1;

   anaflex_options.run_anaflex                              = true;
   anaflex_options.nfrec                                    = 10;
   anaflex_options.instprofiles                             = false;
   anaflex_options.run_mode_1                               = false;
   anaflex_options.run_mode_1_1                             = false;
   anaflex_options.run_mode_1_2                             = false;
   anaflex_options.run_mode_1_3                             = false;
   anaflex_options.run_mode_1_4                             = false;
   anaflex_options.run_mode_1_5                             = false;
   anaflex_options.run_mode_1_7                             = false;
   anaflex_options.run_mode_1_8                             = false;
   anaflex_options.run_mode_1_12                            = false;
   anaflex_options.run_mode_1_13                            = false;
   anaflex_options.run_mode_1_14                            = false;
   anaflex_options.run_mode_1_18                            = true;
   anaflex_options.run_mode_1_20                            = false;
   anaflex_options.run_mode_1_24                            = false;
   anaflex_options.run_mode_2                               = false;
   anaflex_options.run_mode_2_1                             = false;
   anaflex_options.run_mode_2_2                             = false;
   anaflex_options.run_mode_2_3                             = false;
   anaflex_options.run_mode_2_4                             = false;
   anaflex_options.run_mode_2_5                             = false;
   anaflex_options.run_mode_2_7                             = false;
   anaflex_options.run_mode_2_8                             = false;
   anaflex_options.run_mode_2_12                            = false;
   anaflex_options.run_mode_2_13                            = false;
   anaflex_options.run_mode_2_14                            = false;
   anaflex_options.run_mode_2_18                            = true;
   anaflex_options.run_mode_2_20                            = false;
   anaflex_options.run_mode_2_24                            = false;
   anaflex_options.run_mode_3                               = true;
   anaflex_options.run_mode_3_1                             = true;
   anaflex_options.run_mode_3_5                             = false;
   anaflex_options.run_mode_3_9                             = false;
   anaflex_options.run_mode_3_10                            = false;
   anaflex_options.run_mode_3_14                            = false;
   anaflex_options.run_mode_3_15                            = false;
   anaflex_options.run_mode_3_16                            = false;
   anaflex_options.run_mode_4                               = false;
   anaflex_options.run_mode_4_1                             = false;
   anaflex_options.run_mode_4_6                             = false;
   anaflex_options.run_mode_4_7                             = false;
   anaflex_options.run_mode_4_8                             = true;
   anaflex_options.run_mode_9                               = false;
   anaflex_options.ntimc                                    = 21;
   anaflex_options.tmax                                     = (float)1.6e-6;
   anaflex_options.run_mode_3_5_iii                         = 1;
   anaflex_options.run_mode_3_5_jjj                         = 99999;
   anaflex_options.run_mode_3_10_theta                      = 90.0;
   anaflex_options.run_mode_3_10_refractive_index           = (float)1.3312;
   anaflex_options.run_mode_3_10_lambda                     = 633.0;
   anaflex_options.run_mode_3_14_iii                        = 1;
   anaflex_options.run_mode_3_14_jjj                        = 99999;
      
   batch.missing_atoms                                      = 2;
   batch.missing_residues                                   = 2;
   batch.somo                                               = true;
   batch.grid                                               = false;
   batch.hydro                                              = true;
   batch.zeno                                               = false;
   batch.avg_hydro                                          = false;
   batch.avg_hydro_name                                     = "results";
   batch.height                                             = 0;
   batch.width                                              = 0;
   batch.file                                               .clear( );

   path_load_pdb                                            = "";
   path_view_pdb                                            = "";
   path_load_bead_model                                     = "";
   path_view_asa_res                                        = "";
   path_view_bead_model                                     = "";
   path_open_hydro_res                                      = "";
   saxs_options.path_load_saxs_curve                        = "";
   saxs_options.path_load_gnom                              = "";
   saxs_options.path_load_prr                               = "";

   save_params.field                                        .clear( );

   asa.hydrate_probe_radius                                 = 1.4f;
   asa.hydrate_threshold                                    = 10.0f;

   misc.target_e_density                                    = 0e0;
   misc.target_volume                                       = 0e0;
   misc.set_target_on_load_pdb                              = false;
   misc.equalize_radii                                      = false;

   dmd_options.force_chem                                   = true;
   dmd_options.pdb_static_pairs                             = false;
   dmd_options.threshold_pb_pb                              = 5;
   dmd_options.threshold_pb_sc                              = 5;
   dmd_options.threshold_sc_sc                              = 5;

   saxs_options.normalize_by_mw                             = true;

   saxs_options.saxs_iq_native_debye                        = false;
   saxs_options.saxs_iq_native_hybrid                       = false;
   saxs_options.saxs_iq_native_hybrid2                      = false;
   saxs_options.saxs_iq_native_hybrid3                      = true;
   saxs_options.saxs_iq_native_fast                         = false;
   saxs_options.saxs_iq_native_fast_compute_pr              = false;
   saxs_options.saxs_iq_foxs                                = false;
   saxs_options.saxs_iq_crysol                              = false;

   saxs_options.sans_iq_native_debye                        = true;
   saxs_options.sans_iq_native_hybrid                       = false;
   saxs_options.sans_iq_native_hybrid2                      = false;
   saxs_options.sans_iq_native_hybrid3                      = false;
   saxs_options.sans_iq_native_fast                         = false;
   saxs_options.sans_iq_native_fast_compute_pr              = false;
   saxs_options.sans_iq_cryson                              = false;

   saxs_options.hybrid2_q_points                            = 15;

   saxs_options.iq_ask                                      = false;

   saxs_options.iq_scale_ask                                = false;
   saxs_options.iq_scale_angstrom                           = true;
   saxs_options.iq_scale_nm                                 = false;

   saxs_options.sh_max_harmonics                            = 15;
   saxs_options.sh_fibonacci_grid_order                     = 17;
   saxs_options.crysol_hydration_shell_contrast             = 0.03f;
   saxs_options.crysol_default_load_difference_intensity    = true;
   saxs_options.crysol_version_26                           = false;
   saxs_options.crysol_version_3                            = false;
   saxs_options.crysol_water_dummy_beads                    = false;

   saxs_options.fast_bin_size                               = 0.5f;
   saxs_options.fast_modulation                             = 0.23f;

   saxs_options.compute_saxs_coeff_for_bead_models          = true;
   saxs_options.compute_sans_coeff_for_bead_models          = false;

   saxs_options.default_atom_filename                       = "";
   saxs_options.default_hybrid_filename                     = "";
   saxs_options.default_saxs_filename                       = "";
   saxs_options.default_rotamer_filename                    = "";

   saxs_options.steric_clash_distance                       = 20.0;
   saxs_options.steric_clash_recheck_distance               = 0.0;

   saxs_options.disable_iq_scaling                          = false;
   saxs_options.disable_nnls_scaling                        = false;
   saxs_options.autocorrelate                               = true;
   saxs_options.hybrid_radius_excl_vol                      = false;
   saxs_options.scale_excl_vol                              = 1.0f;
   saxs_options.subtract_radius                             = false;
   saxs_options.iqq_scale_minq                              = 0.0f;
   saxs_options.iqq_scale_maxq                              = 0.0f;

   saxs_options.iqq_scale_nnls                              = false;
   saxs_options.iqq_scale_linear_offset                     = false;
   saxs_options.iqq_scale_chi2_fitting                      = true;
   saxs_options.iqq_expt_data_contains_variances            = false;
   saxs_options.iqq_ask_target_grid                         = true;
   saxs_options.iqq_scale_play                              = false;
   saxs_options.swh_excl_vol                                = 0.0f;
   saxs_options.iqq_default_scaling_target                  = "";

   saxs_options.saxs_iq_hybrid_adaptive                     = true;
   saxs_options.sans_iq_hybrid_adaptive                     = true;

   saxs_options.bead_model_rayleigh                         = true;
   saxs_options.iqq_log_fitting                             = false;
   saxs_options.iqq_kratky_fit                              = false;
   saxs_options.iqq_use_atomic_ff                           = false;
   saxs_options.iqq_use_saxs_excl_vol                       = false;
   saxs_options.alt_hydration                               = false;

   saxs_options.xsr_symmop                                  = 2;
   saxs_options.xsr_nx                                      = 32;
   saxs_options.xsr_ny                                      = 32;
   saxs_options.xsr_griddistance                            = 3e0;
   saxs_options.xsr_ncomponents                             = 1;
   saxs_options.xsr_compactness_weight                      = 10e0;
   saxs_options.xsr_looseness_weight                        = 10e0;
   saxs_options.xsr_temperature                             = 1e-3;

   hydro.zeno_zeno                                          = true;
   hydro.zeno_interior                                      = false;
   hydro.zeno_surface                                       = false;
   hydro.zeno_zeno_steps                                    = 1000;
   hydro.zeno_interior_steps                                = 1000;
   hydro.zeno_surface_steps                                 = 1000;
   hydro.zeno_surface_thickness                             = 0.0f;
   hydro.zeno_surface_thickness_from_rg                     = false;

   // linear

   // hydro.zeno_surface_thickness_from_rg_a                = -0.147;
   // hydro.zeno_surface_thickness_from_rg_b                = 0.0328;

   // sigmoid

   hydro.zeno_surface_thickness_from_rg_a                   = 1.071009096;
   hydro.zeno_surface_thickness_from_rg_b                   = 20.85931361;
   hydro.zeno_surface_thickness_from_rg_c                   = 8.013801076;

   misc.hydro_supc                                          = true;
   misc.hydro_zeno                                          = false;

   rotamer_changed                                          = true;  // force on-demand loading of rotamer file

   batch.saxs_search                                        = false;
   batch.zeno                                               = false;

   saxs_options.ignore_errors                               = false;
   saxs_options.trunc_pr_dmax_target                        = false;
   saxs_options.alt_ff                                      = true;
   saxs_options.crysol_explicit_hydrogens                   = false;
   saxs_options.use_somo_ff                                 = false;
   saxs_options.five_term_gaussians                         = true;
   saxs_options.iq_exact_q                                  = false;
   saxs_options.use_iq_target_ev                            = false;
   saxs_options.set_iq_target_ev_from_vbar                  = false;
   saxs_options.iq_target_ev                                = 0e0;
   saxs_options.hydration_rev_asa                           = false;
   saxs_options.compute_exponentials                        = false;
   saxs_options.compute_exponential_terms                   = 5;
   saxs_options.dummy_saxs_name                             = "DAM";
   saxs_options.dummy_saxs_names                            .clear( );
   saxs_options.dummy_saxs_names                            .push_back( saxs_options.dummy_saxs_name );
   saxs_options.multiply_iq_by_atomic_volume                = false;
   saxs_options.dummy_atom_pdbs_in_nm                       = false;
   saxs_options.iq_global_avg_for_bead_models               = false;
   saxs_options.apply_loaded_sf_repeatedly_to_pdb           = false;
   saxs_options.bead_models_use_var_len_sf                  = false;
   saxs_options.bead_models_var_len_sf_max                  = 10;
   saxs_options.bead_models_use_gsm_fitting                 = false;
   saxs_options.bead_models_use_quick_fitting               = true;
   saxs_options.bead_models_use_bead_radius_ev              = true;
   saxs_options.bead_models_rho0_in_scat_factors            = true;
   saxs_options.smooth                                      = 0;
   saxs_options.ev_exp_mult                                 = 1e0;
   saxs_options.sastbx_method                               = 0;
   saxs_options.saxs_iq_sastbx                              = false;
   saxs_options.saxs_iq_native_sh                           = false;
   saxs_options.sans_iq_native_sh                           = false;

   saxs_options.alt_sh1                                     = false;
   saxs_options.alt_sh2                                     = false;

   grid.create_nmr_bead_pdb                                 = false;

   batch.compute_iq_only_avg                                = false;

   asa.vvv                                                  = false;
   asa.vvv_probe_radius                                     = 1.4f;
   asa.vvv_grid_dR                                          = 0.5f;

   misc.export_msroll                                       = false;
   misc.parallel_grpy                                       = false;
   misc.auto_calc_hydro_method                              = AUTO_CALC_HYDRO_ZENO;

   saxs_options.cs_qRgmax                                   = 1e0;
   saxs_options.cs_qstart                                   = saxs_options.qstart * saxs_options.qstart;
   saxs_options.cs_qend                                     = saxs_options.qend   * saxs_options.qend;

   saxs_options.conc                                        = 1e0;
   saxs_options.psv                                         = 7.2e-1;
   saxs_options.use_cs_psv                                  = false;
   saxs_options.cs_psv                                      = 0e0;
   saxs_options.I0_exp                                      = 5.4e-5;
   saxs_options.I0_theo                                     = 1.633e-2;
   saxs_options.diffusion_len                               = 2.82e-13;
   saxs_options.nucleon_mass                                = 1.6606e-24; // update 20 jan 2024 from 1.674e-24;

   saxs_options.guinier_outlier_reject                      = false;
   saxs_options.guinier_outlier_reject_dist                 = 2e0;
   saxs_options.guinier_use_sd                              = false;
   saxs_options.guinier_use_standards                       = false;

   saxs_options.cryson_sh_max_harmonics                     = 15;
   saxs_options.cryson_sh_fibonacci_grid_order              = 17;
   saxs_options.cryson_hydration_shell_contrast             = 
      saxs_options.d2o_conc * saxs_options.d2o_scat_len_dens +
      ( 1e0 - saxs_options.d2o_conc ) * ( saxs_options.h2o_scat_len_dens );
   saxs_options.cryson_manual_hs                            = false;

   advanced_config.temp_dir_threshold_mb                    = 50;

   gparams                                                  .clear( );
   gparams[ "guinier_auto_fit" ]                            = "1";
   gparams[ "perdeuteration" ]                              = "0";
   gparams[ "guinier_qRtmax" ]                              = "1";
   gparams[ "guinier_electron_nucleon_ratio" ]              = "1.87e0";

   gparams[ "guinier_mwt_k" ]                               = "1";
   gparams[ "guinier_mwt_c" ]                               = "-2.095";
   gparams[ "guinier_mwt_qmax" ]                            = "0.2";

   gparams[ "hplc_bl_linear"             ]                  = "false";
   gparams[ "hplc_bl_integral"           ]                  = "true";
   gparams[ "hplc_bl_save"               ]                  = "false";
   gparams[ "hplc_bl_smooth"             ]                  = "0";
   gparams[ "hplc_bl_reps"               ]                  = "5";
   gparams[ "hplc_zi_window"             ]                  = "25";
   gparams[ "hplc_discard_it_sd_mult"    ]                  = "2";
   gparams[ "hplc_cb_discard_it_sd_mult" ]                  = "true";
   gparams[ "hplc_dist_max"              ]                  = "50.0";
   gparams[ "hplc_cormap_maxq"           ]                  = "0.05";
   gparams[ "hplc_cormap_alpha"          ]                  = "0.01";
   gparams[ "save_csv_on_load_pdb"       ]                  = "false";
   gparams[ "hplc_cb_makeiq_cutmax_pct"  ]                  = "true";
   gparams[ "hplc_makeiq_cutmax_pct"     ]                  = "1";
   gparams[ "hplc_cb_makeiq_avg_peaks"   ]                  = "false";
   gparams[ "hplc_makeiq_avg_peaks"      ]                  = "5";
   gparams[ "zeno_repeats"               ]                  = "1";
   gparams[ "zeno_max_cap"               ]                  = "false";
   gparams[ "zeno_max_cap_pct"           ]                  = "0.5";

   gparams[ "covolume"                   ]                  = "12.4";
   gparams[ "use_pH"                     ]                  = "true";
   gparams[ "thresh_SS"                  ]                  = "2.5";
   gparams[ "thresh_carb_O"              ]                  = "2";
   gparams[ "thresh_carb_N"              ]                  = "2";

   gparams[ "vdw_inflate"                ]                  = "false";
   gparams[ "vdw_inflate_mult"           ]                  = "1";

   save_params_force_results_name( save_params );
}

void US_Hydrodyn::set_default()
{
   QFile f;
   QString str;
   int j;
   // only keep one copy of defaults in system root dir
   f.setFileName( US_Config::get_home_dir() + "etc/somo.defaults");
   bool config_read = false;

   if (f.open(QIODevice::ReadOnly)) // read system directory
   {
      j=read_config(f);
      if ( j )
      {
         QTextStream( stdout ) << "read config returned " << j << Qt::endl;
         if ( init_configs_silently ) {
            qDebug() <<
               QString( "The somo.default configuration file %1 was found to be corrupt, resorting to hard-coded defaults" )
               .arg( f.fileName() );
         } else {
            US_Static::us_message(us_tr("Please note:"),
                                  us_tr("The somo.default configuration file was found to be corrupt.\n"
                                        "Resorting to hard-coded defaults."));
         }
      }
      else
      {
         config_read = true;
      }
   }
   else
   {
      if ( init_configs_silently ) {
         qDebug() <<
            QString( "The somo.default configuration file %1 was not found, resorting to hard-coded defaults" )
            .arg( f.fileName() );
      } else {
         US_Static::us_message(us_tr("Notice:"),
                               us_tr("Configuration defaults file ") +
                               f.fileName() + us_tr(" not found\nUsing hard-coded defaults."));
      }
   }

   if ( !config_read )
   {
      hard_coded_defaults();
   }

   // defaults that SHOULD NOT BE MOVED INTO somo.config

   if ( pdb_vis.filename.isEmpty() )
   {
      pdb_vis.filename = USglobal->config_list.system_dir + "/etc/rasmol.spt"; //default color file
   }

   if ( saxs_options.default_atom_filename.isEmpty() )
   {
      saxs_options.default_atom_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.atom";
   }
   if ( saxs_options.default_hybrid_filename.isEmpty() )
   {
      saxs_options.default_hybrid_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hybrid";
   }
   if ( saxs_options.default_saxs_filename.isEmpty() )
   {
      saxs_options.default_saxs_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.saxs_atoms";
   }
   if ( saxs_options.default_rotamer_filename.isEmpty() )
   {
      saxs_options.default_rotamer_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hydrated_rotamer";
   }
   if ( saxs_options.default_ff_filename.isEmpty() )
   {
      saxs_options.default_ff_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.ff";
   }

   rotamer_changed = true;  // force on-demand loading of rotamer file

   save_params_force_results_name( save_params );

   default_sidechain_overlap = sidechain_overlap;
   default_mainchain_overlap = mainchain_overlap;
   default_buried_overlap = buried_overlap;
   default_grid_exposed_overlap = grid_exposed_overlap;
   default_grid_buried_overlap = grid_buried_overlap;
   default_grid_overlap = grid_overlap;
   default_bead_output = bead_output;
   default_asa = asa;
   default_misc = misc;
   default_overlap_tolerance = overlap_tolerance;
   default_hydro = hydro;
   default_pdb_vis = pdb_vis;
   default_pdb_parse = pdb_parse;
   default_grid = grid;
   default_saxs_options = saxs_options;
   default_batch = batch;
   default_save_params = save_params;
   default_bd_options = bd_options;
   default_anaflex_options = anaflex_options;
   default_gparams         = gparams;
}

QString US_Hydrodyn::default_differences_load_pdb()
{
   QString str = "";
   // not supported, could overload ==
   // if ( pdb_parse == default_pdb_parse )
   // {
   //   return str;
   // }
   QString base = "PDB Options -> ";
   QString sub = "Parsing -> ";
   if ( pdb_parse.skip_hydrogen != default_pdb_parse.skip_hydrogen )
   {
      str += QString(base + sub + "Skip hydrogen: %1.\n")
         .arg(pdb_parse.skip_hydrogen ? "Selected" : "Not selected");
   }
   if ( pdb_parse.skip_water != default_pdb_parse.skip_water )
   {
      str += QString(base + sub + "Skip water: %1.\n")
         .arg(pdb_parse.skip_water ? "Selected" : "Not selected");
   }
   if ( pdb_parse.alternate != default_pdb_parse.alternate )
   {
      str += QString(base + sub + "Skip alternate conformations: %1.\n")
         .arg(pdb_parse.alternate ? "Selected" : "Not selected");
   }
   if ( pdb_parse.find_sh != default_pdb_parse.find_sh )
   {
      str += QString(base + sub + "Find free SH, change residue coding: %1.\n")
         .arg(pdb_parse.find_sh ? "Selected" : "Not selected");
   }
   if ( default_gparams.count( "thresh_SS" ) && gparams.count( "thresh_SS" ) &&
        default_gparams[ "thresh_SS" ].toDouble() != gparams[ "thresh_SS" ].toDouble() ) {
      str += QString(base + sub + "Find free SH, SS bond distance threshold [A]: %1.\n")
         .arg( gparams[ "thresh_SS" ].toDouble() );
   }
   if ( advanced_config.experimental_renum ) 
   {
      str += us_tr( "Renumber PDB residues on load is on. (Experimental)" );
   }

   if ( gparams[ "save_csv_on_load_pdb" ] != default_gparams[ "save_csv_on_load_pdb" ] )
   {
      str += QString(base + sub + "Save CSV on load PDB: %1\n")
         .arg( gparams[ "save_csv_on_load_pdb" ] == "true" ? "Selected" : "Not selected");
   }
   if ( pdb_parse.missing_residues != default_pdb_parse.missing_residues )
   {
      QString opt = "Unknown state";
      switch ( pdb_parse.missing_residues )
      {
      case 0 :
         opt = "List them and stop operation";
         break;
      case 1 :
         opt = "List them, skip residue and proceed";
         break;
      case 2 :
         opt = "Use automatic bead builder (approximate method)";
         break;
      }
      str += QString(base + sub + "If non-coded residues are found: %1.\n")
         .arg(opt);
   }
   if ( pdb_parse.missing_atoms != default_pdb_parse.missing_atoms )
   {
      QString opt = "Unknown state";
      switch ( pdb_parse.missing_atoms )
      {
      case 0 :
         opt = "List them and stop operation";
         break;
      case 1 :
         opt = "List them, skip residue and proceed";
         break;
      case 2 :
         opt = "Use automatic bead builder (approximate method)";
         break;
      }
      str += QString(base + sub + "If missing atoms within a residue are found: %1.\n")
         .arg(opt);
   }
   return str;
}

QString US_Hydrodyn::default_differences_somo()
{
   QString str = "";
   QString base = "SOMO Options -> ";
   QString sub = "ASA Calculation -> ";
   if ( asa.calculation != default_asa.calculation )
   {
      str += QString(base + sub + "Perform ASA calculation: %1.\n")
         .arg(asa.calculation ? "On" : "Off");
   }
   if ( asa.recheck_beads != default_asa.recheck_beads )
   {
      str += QString(base + sub + "Recheck bead ASA: %1.\n")
         .arg(asa.recheck_beads ? "On" : "Off");
   }
   if ( asa.method != default_asa.method )
   {
      str += QString(base + sub + "ASA method: %1.\n")
         .arg(asa.method ? "Rolling Sphere" : "Voronoi Tesselation");
   }
   if ( asa.probe_radius != default_asa.probe_radius )
   {
      str += QString(base + sub + "ASA Probe Radius (A): %1\n").arg(asa.probe_radius);
   }
   if ( asa.probe_recheck_radius != default_asa.probe_recheck_radius &&
        asa.recheck_beads )
   {
      str += QString(base + sub + "Probe Recheck Radius (A): %1\n").arg(asa.probe_recheck_radius);
   }
   if ( asa.threshold != default_asa.threshold )
   {
      str += QString(base + sub + "SOMO ASA Threshold (A^2): %1\n").arg(asa.threshold);
   }
   if ( asa.threshold_percent != default_asa.threshold_percent )
   {
      str += QString(base + sub + "SOMO Bead ASA Threshold %: %1\n").arg(asa.threshold_percent);
   }
   if ( asa.grid_threshold != default_asa.grid_threshold )
   {
      str += QString(base + sub + "Grid ASA Threshold (A^2): %1\n").arg(asa.grid_threshold);
   }
   if ( asa.grid_threshold_percent != default_asa.grid_threshold_percent )
   {
      str += QString(base + sub + "Grid Bead ASA Threshold %: %1\n").arg(asa.grid_threshold_percent);
   }
   if ( asa.asab1_step != default_asa.asab1_step &&
        asa.method )
   {
      str += QString(base + sub + "ASAB1 Step Size (A): %1\n").arg(asa.asab1_step);
   }

   sub = "SoMo Overlap Reduction -> ";
   if ( overlap_tolerance != default_overlap_tolerance )
   {
      str += QString(base + sub + "Bead Overlap Tolerance: %1\n").arg(overlap_tolerance);
   }

   QString sub2 = "Exposed Side Chain Beads -> ";
   if ( sidechain_overlap.fuse_beads != default_sidechain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(sidechain_overlap.fuse_beads ? "On" : "Off");
   }
   if ( sidechain_overlap.fuse_beads_percent != default_sidechain_overlap.fuse_beads_percent &&
        sidechain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(sidechain_overlap.fuse_beads_percent);
   }
   if ( sidechain_overlap.remove_overlap != default_sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(sidechain_overlap.remove_overlap ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_sync != default_sidechain_overlap.remove_sync &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(sidechain_overlap.remove_sync ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_sync_percent != default_sidechain_overlap.remove_sync_percent &&
        sidechain_overlap.remove_overlap && sidechain_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(sidechain_overlap.remove_sync_percent);
   }
   if ( sidechain_overlap.remove_hierarch != default_sidechain_overlap.remove_hierarch &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(sidechain_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_hierarch_percent != default_sidechain_overlap.remove_hierarch_percent &&
        sidechain_overlap.remove_overlap && sidechain_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(sidechain_overlap.remove_hierarch_percent);
   }
   if ( sidechain_overlap.translate_out != default_sidechain_overlap.translate_out &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Outward Translation: %1\n")
         .arg(sidechain_overlap.translate_out ? "On" : "Off");
   }

   sub2 = "Exposed Main and Side Chain Beads -> ";
   if ( mainchain_overlap.fuse_beads != default_mainchain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(mainchain_overlap.fuse_beads ? "On" : "Off");
   }
   if ( mainchain_overlap.fuse_beads_percent != default_mainchain_overlap.fuse_beads_percent &&
        mainchain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(mainchain_overlap.fuse_beads_percent);
   }
   if ( mainchain_overlap.remove_overlap != default_mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(mainchain_overlap.remove_overlap ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_sync != default_mainchain_overlap.remove_sync &&
        mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(mainchain_overlap.remove_sync ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_sync_percent != default_mainchain_overlap.remove_sync_percent &&
        mainchain_overlap.remove_overlap && mainchain_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(mainchain_overlap.remove_sync_percent);
   }
   if ( mainchain_overlap.remove_hierarch != default_mainchain_overlap.remove_hierarch  &&
        mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(mainchain_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_hierarch_percent != default_mainchain_overlap.remove_hierarch_percent &&
        mainchain_overlap.remove_overlap && mainchain_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(mainchain_overlap.remove_hierarch_percent);
   }

   sub2 = "Buried Beads -> ";
   if ( buried_overlap.fuse_beads != default_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(buried_overlap.fuse_beads ? "On" : "Off");
   }
   if ( buried_overlap.fuse_beads_percent != default_buried_overlap.fuse_beads_percent &&
        buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(buried_overlap.fuse_beads_percent);
   }
   if ( buried_overlap.remove_overlap != default_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(buried_overlap.remove_overlap ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync != default_buried_overlap.remove_sync && 
        buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(buried_overlap.remove_sync ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync_percent != default_buried_overlap.remove_sync_percent &&
        buried_overlap.remove_overlap && buried_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(buried_overlap.remove_sync_percent);
   }
   if ( buried_overlap.remove_hierarch != default_buried_overlap.remove_hierarch &&
        buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(buried_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( buried_overlap.remove_hierarch_percent != default_buried_overlap.remove_hierarch_percent &&
        buried_overlap.remove_overlap && buried_overlap.remove_hierarch )
   {
      str += QString(base + sub + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(buried_overlap.remove_hierarch_percent);
   }
   return str;
}

QString US_Hydrodyn::default_differences_grid()
{
   QString str = "";

   QString base = "SOMO Options -> ";
   QString sub = "AtoB (Grid) Overlap Reduction -> ";
   QString sub2 = "Exposed Grid Beads -> ";
   if ( grid_exposed_overlap.fuse_beads != default_grid_exposed_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_exposed_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_exposed_overlap.fuse_beads_percent != default_grid_exposed_overlap.fuse_beads_percent &&
        grid_exposed_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_exposed_overlap.fuse_beads_percent);
   }
   if ( grid_exposed_overlap.remove_overlap != default_grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_exposed_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_sync != default_grid_exposed_overlap.remove_sync &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_exposed_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_sync_percent != default_grid_exposed_overlap.remove_sync_percent &&
        grid_exposed_overlap.remove_overlap && grid_exposed_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_exposed_overlap.remove_sync_percent);
   }
   if ( grid_exposed_overlap.remove_hierarch != default_grid_exposed_overlap.remove_hierarch &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_exposed_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_hierarch_percent != default_grid_exposed_overlap.remove_hierarch_percent &&
        grid_exposed_overlap.remove_overlap && grid_exposed_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_exposed_overlap.remove_hierarch_percent);
   }
   if ( grid_exposed_overlap.translate_out != default_grid_exposed_overlap.translate_out &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Outward Translation: %1\n")
         .arg(grid_exposed_overlap.translate_out ? "On" : "Off");
   }

   sub2 = "Buried Grid Beads -> ";
   if ( grid_buried_overlap.fuse_beads != default_grid_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_buried_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_buried_overlap.fuse_beads_percent != default_grid_buried_overlap.fuse_beads_percent &&
        grid_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_buried_overlap.fuse_beads_percent);
   }
   if ( grid_buried_overlap.remove_overlap != default_grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_buried_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_sync != default_grid_buried_overlap.remove_sync &&
        grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_buried_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_sync_percent != default_grid_buried_overlap.remove_sync_percent &&
        grid_buried_overlap.remove_overlap && grid_buried_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_buried_overlap.remove_sync_percent);
   }
   if ( grid_buried_overlap.remove_hierarch != default_grid_buried_overlap.remove_hierarch &&
        grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_buried_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_hierarch_percent != default_grid_buried_overlap.remove_hierarch_percent &&
        grid_buried_overlap.remove_overlap && grid_buried_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_buried_overlap.remove_hierarch_percent);
   }

   sub2 = "Grid Beads -> ";
   if ( grid_overlap.fuse_beads != default_grid_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_overlap.fuse_beads_percent != default_grid_overlap.fuse_beads_percent &&
        grid_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_overlap.fuse_beads_percent);
   }
   if ( grid_overlap.remove_overlap != default_grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_overlap.remove_sync != default_grid_overlap.remove_sync &&
        grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_overlap.remove_sync_percent != default_grid_overlap.remove_sync_percent &&
        grid_overlap.remove_overlap && grid_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_overlap.remove_sync_percent);
   }
   if ( grid_overlap.remove_hierarch != default_grid_overlap.remove_hierarch &&
        grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_overlap.remove_hierarch_percent != default_grid_overlap.remove_hierarch_percent &&
        grid_overlap.remove_overlap && grid_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_overlap.remove_hierarch_percent);
   }

   base = "Grid Functions (AtoB) -> ";
   if ( grid.center != default_grid.center )
   {
      str += QString(base +  "Computations Relative to: Center of %1\n")
         .arg(grid.center ? ( grid.center == 2 ? "Scattering intensity" : "Cublet" ) : "Mass");
   }
   if ( grid.cube_side != default_grid.cube_side )
   {
      str += QString(base + "Cube Side (Angstrom): %1\n").arg(grid.cube_side);
   }
   if ( grid.cubic != default_grid.cubic )
   {
      str += QString(base + "Apply Cubic Grid: %1\n")
         .arg(grid.cubic ? "On" : "Off");
   }
   if ( grid.hydrate != default_grid.hydrate )
   {
      str += QString(base + " Add theoretical hydration (PDB only): %1\n")
         .arg(grid.hydrate ? "On" : "Off");
   }
   if ( grid.tangency != default_grid.tangency )
   {
      str += QString(base + "Expand Beads to Tangency: %1\n")
         .arg(grid.tangency ? "On" : "Off");
   }
   if ( grid.enable_asa != default_grid.enable_asa )
   {
      str += QString(base + "Enable ASA screening: %1\n")
         .arg(grid.enable_asa ? "On" : "Off");
   }
   return str;
}

QString US_Hydrodyn::default_differences_hydro()
{
   QString str = "";
   QString base = "SOMO Options -> Hydrodynamic Calculations -> ";
   if ( hydro.unit != default_hydro.unit )
   {
      str += QString(base + "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : %1\n").arg(-hydro.unit);
   }
   if ( hydro.solvent_name != default_hydro.solvent_name )
   {
      str += QString(base + "Solvent name: %1\n").arg(hydro.solvent_name);
   }
   if ( hydro.solvent_acronym != default_hydro.solvent_acronym )
   {
      str += QString(base + "Solvent acronym: %1\n").arg(hydro.solvent_acronym);
   }
   if ( hydro.temperature != default_hydro.temperature )
   {
      str += QString(base + "Temperature (C): %1\n").arg(hydro.temperature);
   }
   if ( hydro.pH != default_hydro.pH )
   {
      str += QString(base + "pH: %1\n").arg(hydro.pH);
   }
   if ( hydro.solvent_viscosity != default_hydro.solvent_viscosity )
   {
      str += QString(base + "Solvent viscosity (cP): %1\n").arg(hydro.solvent_viscosity);
   }
   if ( hydro.solvent_density != default_hydro.solvent_density )
   {
      str += QString(base + "Solvent density (g/ml): %1\n").arg(hydro.solvent_density);
   }
   if ( hydro.reference_system != default_hydro.reference_system )
   {
      str += QString(base + "Computations Relative to: %1\n")
         .arg(hydro.reference_system ? "Cartesian Origin" : "Diffusion Center");
   }
   if ( hydro.boundary_cond != default_hydro.boundary_cond )
   {
      str += QString(base + "Boundary Conditions: %1\n")
         .arg(hydro.boundary_cond ? "Slip" : "Stick");
   }
   if ( hydro.mass_correction != default_hydro.mass_correction )
   {
      str += QString(base + "Total Mass of Model: %1\n")
         .arg(hydro.mass_correction ? "Manual" : "Automatic");
   }
   if ( hydro.mass != default_hydro.mass &&
        hydro.mass_correction )
   {
      str += QString(base + "Entered Mass: %1\n").arg(hydro.mass);
   }
   if ( hydro.volume_correction != default_hydro.volume_correction )
   {
      str += QString(base + "Total Volume of Model: %1\n")
         .arg(hydro.volume_correction ? "Manual" : "Automatic");
   }
   if ( hydro.use_avg_for_volume != default_hydro.use_avg_for_volume )
   {
      str += QString(base + "Use ASA for volume: %1\n")
         .arg(hydro.use_avg_for_volume ? "On" : "Off");
   }
   if ( hydro.volume != default_hydro.volume &&
        hydro.volume_correction )
   {
      str += QString(base + "Entered Volume: %1\n").arg(hydro.volume);
   }
   if ( hydro.bead_inclusion != default_hydro.bead_inclusion )
   {
      str += QString(base + "Inclusion of Buried Beads in Hydrodynamic Calculations: %1\n")
         .arg(hydro.bead_inclusion ? "Include" : "Exclude");
   }
   if ( hydro.grpy_bead_inclusion != default_hydro.grpy_bead_inclusion )
   {
      str += QString(base + "Inclusion of Buried Beads in GRPY Hydrodynamic Calculations: %1\n")
         .arg(hydro.grpy_bead_inclusion ? "Include" : "Exclude");
   }
   if ( hydro.rotational != default_hydro.rotational &&
        !hydro.bead_inclusion )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Rotational Diffusion: %1\n")
         .arg(hydro.rotational ? "Include" : "Exclude" );
   }
   if ( hydro.viscosity != default_hydro.viscosity  &&
        !hydro.bead_inclusion )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Intrinsic Viscosity: %1\n")
         .arg(hydro.viscosity ? "Include" : "Exclude");
   }
   if ( hydro.overlap_cutoff != default_hydro.overlap_cutoff )
   {
      str += QString(base + "Overlap cut-off: %1\n")
         .arg(hydro.overlap_cutoff ? "Manual" : "From Bead Model");
   }
   if ( hydro.overlap != default_hydro.overlap &&
        hydro.overlap_cutoff )
   {
      str += QString(base + "Entered overlap cutoff: %1\n").arg(hydro.overlap);
   }

   if ( hydro.overlap != default_hydro.overlap &&
        hydro.overlap_cutoff )
   {
      str += QString(base + "Entered overlap cutoff: %1\n").arg(hydro.overlap);
   }

   return str;
}

QString US_Hydrodyn::default_differences_main()
{
   QString str = "";
   QString base = "Main Window -> PDB Functions -> ";
   
   // qDebug() << "use_pH default " << ( default_gparams.count( "use_pH" ) ? QString( "%1" ).arg( default_gparams[ "use_pH" ] == "true" ? "true" : "false" ) : QString( "not set" ) );
   // qDebug() << "use_pH current " << ( gparams.count( "use_pH" ) ? QString( "%1" ).arg( gparams[ "use_pH" ] == "true" ? "true" : "false" ) : QString( "not set" ) );
   
   if ( default_gparams.count( "use_pH" ) && gparams.count( "use_pH" ) &&
        default_gparams[ "use_pH" ] != gparams[ "use_pH" ] ) {
      str += QString(base + "pH dependent ionization and psv calculations %1.\n")
         .arg( gparams[ "use_pH" ] == "true" ? "enabled" : "disabled" );
   }

   return str;
}

QString US_Hydrodyn::default_differences_misc()
{
   QString str = "";
   QString base = "SOMO Options -> Miscellaneous Options -> ";

   if ( misc.compute_vbar != default_misc.compute_vbar )
   {
      str += QString(base + "Calculate vbar: %1\n")
         .arg(misc.compute_vbar ? "On" : "Off");
   }
   if ( misc.vbar != default_misc.vbar &&
        !misc.compute_vbar )
   {
      str += QString(base + "Entered vbar value: %1\n").arg(misc.vbar);
   }
   if ( default_gparams.count( "covolume" ) && gparams.count( "covolume" ) &&
        default_gparams[ "covolume" ].toDouble() != gparams[ "covolume" ].toDouble() ) {
      str += QString(base + "covolume [cm^3/mol] %1.\n")
         .arg( gparams[ "covolume" ].toDouble() );
   }
   if ( misc.vbar_temperature != default_misc.vbar_temperature &&
        !misc.compute_vbar )
   {
      str += QString(base + "Vbar measured/computed at T= %1\n").arg(misc.vbar_temperature);
   }
   if ( ( misc.pb_rule_on || misc.restore_pb_rule ) != default_misc.pb_rule_on )
   {
      str += QString(base + "Peptide bond rule: %1\n")
         .arg(misc.pb_rule_on ? "On" : "Off");
   }
   if ( misc.hydrovol != default_misc.hydrovol )
   {
      str += QString(base + "Hydration Water Vol. (A^3): %1\n").arg(misc.hydrovol);
   }
   QString sub = " Automatic Bead Builder -> Average ";
   if ( misc.avg_radius != default_misc.avg_radius )
   {
      str += QString(base + sub + "atomic radius (A): %1\n").arg(misc.avg_radius);
   }
   if ( misc.avg_mass != default_misc.avg_mass )
   {
      str += QString(base + sub + "atomic mass (Da): %1\n").arg(misc.avg_mass);
   }
   if ( misc.avg_num_elect != default_misc.avg_num_elect )
   {
      str += QString(base + sub + "atomic number of electrons: %1\n").arg(misc.avg_num_elect);
   }
   if ( misc.avg_protons != default_misc.avg_protons )
   {
      str += QString(base + sub + "atomic number of protons: %1\n").arg(misc.avg_protons);
   }
   if ( misc.avg_hydration != default_misc.avg_hydration )
   {
      str += QString(base + sub + "atomic hydration: %1\n").arg(misc.avg_hydration);
   }
   if ( misc.avg_volume != default_misc.avg_volume )
   {
      str += QString(base + sub + "bead/atom volume (A^3): %1\n").arg(misc.avg_volume);
   }
   if ( misc.avg_vbar != default_misc.avg_vbar )
   {
      str += QString(base + sub + "Residue vbar: %1\n").arg(misc.avg_vbar);
   }
   // if ( misc.hydro_zeno )
   // {
   //    str += QString(base + "Hydrodynamic method Zeno\n" );
   // }
   return str;
}

QString US_Hydrodyn::default_differences_saxs_options()
{
   QString str = "";
   QString base = "SOMO Options -> SAXS/SANS Options -> ";
   if ( saxs_options.water_e_density != default_saxs_options.water_e_density )
   {
      str += QString(base + "Water electron density value: %1\n").arg(saxs_options.water_e_density );
   }

   if ( saxs_options.h_scat_len != default_saxs_options.h_scat_len )
   {
      str += QString(base + "H scattering length (*10^-12 cm): %1\n").arg(saxs_options.h_scat_len);
   }
   if ( saxs_options.d_scat_len != default_saxs_options.d_scat_len )
   {
      str += QString(base + "*10^-12 cm): %1\n").arg(saxs_options.d_scat_len);
   }
   if ( saxs_options.h2o_scat_len_dens != default_saxs_options.h2o_scat_len_dens )
   {
      str += QString(base + "H2O scattering length density (*10^-10 cm^2): %1\n").arg(saxs_options.h2o_scat_len_dens);
   }
   if ( saxs_options.d2o_scat_len_dens != default_saxs_options.d2o_scat_len_dens )
   {
      str += QString(base + "D2O scattering length density (*10^-10 cm^2): %1\n").arg(saxs_options.d2o_scat_len_dens);
   }
   if ( saxs_options.d2o_conc != default_saxs_options.d2o_conc )
   {
      str += QString(base + "Buffer D2O fraction (0 to 1): %1\n").arg(saxs_options.d2o_conc);
   }
   if ( saxs_options.frac_of_exch_pep != default_saxs_options.frac_of_exch_pep )
   {
      str += QString(base + "Fraction of non-exchanged peptide H (0 to 1): %1\n").arg(saxs_options.frac_of_exch_pep);
   }

   if ( saxs_options.wavelength != default_saxs_options.wavelength )
   {
      str += QString(base + "Entered wavelength value: %1\n").arg(saxs_options.wavelength);
   }
   if ( saxs_options.start_angle != default_saxs_options.start_angle )
   {
      str += QString(base + "Start Angle: %1\n").arg(saxs_options.start_angle );
   }
   if ( saxs_options.end_angle != default_saxs_options.end_angle )
   {
      str += QString(base + "Ending Angle: %1\n").arg(saxs_options.end_angle );
   }
   if ( saxs_options.delta_angle != default_saxs_options.delta_angle )
   {
      str += QString(base + "Angle Stepsize: %1\n").arg(saxs_options.delta_angle );
   }
   if ( saxs_options.max_size != default_saxs_options.max_size )
   {
      str += QString(base + "Maximum size: %1\n").arg(saxs_options.max_size );
   }
   if ( saxs_options.bin_size != default_saxs_options.bin_size )
   {
      str += QString(base + "Bin size: %1\n").arg(saxs_options.bin_size );
   }
   if ( saxs_options.hydrate_pdb != default_saxs_options.hydrate_pdb )
   {
      str += QString(base + "Hydrate Original PDB Model: %1\n")
         .arg(saxs_options.hydrate_pdb ? "On" : "Off");
   }
   //   if ( saxs_options.curve != default_saxs_options.curve )
   //   {
   //      str += QString(base + "Curve type: %1\n")
   //         .arg(
   //              saxs_options.curve ? ((saxs_options.curve == 1) ? "SAXS" : "SANS") : "Raw");
   //   }
   //   if ( saxs_options.saxs_sans != default_saxs_options.saxs_sans )
   //   {
   //      str += QString(base + "SAXS or SANS mode: %1\n")
   //         .arg(saxs_options.saxs_sans ? "SANS" : "SAXS");
   //   }

   QString sub = "Guinier options -> ";
   {
      QStringList options;
      options 
         << "guinier_mwt_k"
         << "guinier_mwt_c"
         << "guinier_mwt_qmax"
         ;

      QStringList options_desc;
      options_desc
         << "MW[RT] k"
         << "MW[RT] c"
         << "MW[RT] qmax cut-off"
         ;
         
      for ( int i = 0; i < (int) options.size(); ++i ) {
         if ( gparams[ options[ i ] ] != default_gparams[ options[ i ] ] ) {
            str += QString(base + sub + "%1: %2\n")
               .arg( options_desc[ i ] )
               .arg( gparams[ options[ i ] ] );
         }
      }
   }

   return str;
}

void US_Hydrodyn::display_default_differences()
{
   QString str =
      default_differences_main() +
      default_differences_misc() +
      default_differences_load_pdb() +
      default_differences_somo() +
      default_differences_hydro() +
      default_differences_grid() +
      default_differences_saxs_options();

   if ( str != "" )
   {
      editor_msg( "dark red", "\nNon-default options:\n" + str );
      editor_msg( "black", "\nTo reset to default: Menu bar -> Configuration -> Reset to Default Configuration\n" );
   }
   else
   {
      editor_msg( "dark green", "\nAll options set to default values\n");
   }
   le_bead_model_suffix->setText(
                                 setSuffix
                                 ? (
                                    "<center>"
                                    + getExtendedSuffix(true, true)
                                    + " / "
                                    + getExtendedSuffix(true, false)
                                    + "</center>"
                                    + "<center>"
                                    + getExtendedSuffix(true, true, true)
                                    + " / "
                                    + getExtendedSuffix(true, true, true, true ) 
                                    + "</center>"
                                    )
                                 : "");
}

void US_Hydrodyn::config()
{
   QStringList menu_opts;
   static int last_menu = 0;

   menu_opts
      << us_tr( "Lookup Tables -> Add/Edit Hybridization" ) // hybrid()
      << us_tr( "Lookup Tables -> Add/Edit Atom" ) // edit_atom()
      << us_tr( "Lookup Tables -> Add/Edit Residue" ) // residue()
      << us_tr( "Lookup Tables -> Add/Edit SAXS coefficients" ) // saxs()
      << us_tr( "SOMO Options -> ASA Calculation" ) // show_asa()
      << us_tr( "SOMO Options -> SoMo Overlap Reduction" ) // show_overlap()
      << us_tr( "SOMO Options -> AtoB (Grid) Overlap Reduction" ) // show_grid_overlap()
      << us_tr( "SOMO Options -> Hydrodynamic Calculations" ) // show_hydro()
      << us_tr( "SOMO Options -> Hydrodynamic Calculations Zeno" ) // show_zeno_options()
      << us_tr( "SOMO Options -> Miscellaneous Options" ) // show_misc()
      << us_tr( "SOMO Options -> Bead Model Output" ) // show_bead_output()
      << us_tr( "SOMO Options -> Grid Functions (AtoB)" ) // show_grid()
      << us_tr( "SOMO Options -> SAXS/SANS Options" ) // show_saxs_options()
      // << us_tr( "MD Options -> DMD Options" ) // show_dmd_options()
      << us_tr( "MD Options -> Browflex Options" ) // show_bd_options()
      << us_tr( "MD Options -> Anaflex Options" ) // show_anaflex_options()
      << us_tr( "PDB Options -> Parsing" ) // pdb_parsing()
      << us_tr( "PDB Options -> Visualization" ) // pdb_visualization()
      << us_tr( "Configuration -> Load Configuration" ) // load_config()
      << us_tr( "Configuration -> Save Current Configuration" ) // write_config()
      << us_tr( "Configuration -> Reset to Default Configuration" ) // reset()
      << us_tr( "Configuration -> Advanced Configuration" ) // show_advanced_config()
      << us_tr( "Configuration -> System Configuration" ) // run_us_config()
      // << us_tr( "Configuration -> Administrator" ) // run us_admin()
      ;

   bool ok;
   QString res = US_Static::getItem(
                                       us_tr( "US-SOMO Configuration Options" ), 
                                       us_tr( "Make a selection or press CANCEL" ),
                                       menu_opts, 
                                       last_menu, 
                                       true, 
                                       &ok,
                                       this );

   if ( ok ) {
      int pos   = 0;
      if ( res == us_tr( "Lookup Tables -> Add/Edit Hybridization" ) )
      {
         last_menu = pos;
         hybrid();
      }
      pos++;
      if ( res == us_tr( "Lookup Tables -> Add/Edit Atom" ) )
      {
         last_menu = pos;
         edit_atom();
      }
      pos++;
      if ( res == us_tr( "Lookup Tables -> Add/Edit Residue" ) )
      {
         last_menu = pos;
         residue();
      }
      pos++;
      if ( res == us_tr( "Lookup Tables -> Add/Edit SAXS coefficients" ) )
      {
         last_menu = pos;
         saxs();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> ASA Calculation" ) )
      {
         last_menu = pos;
         show_asa();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> SoMo Overlap Reduction" ) )
      {
         last_menu = pos;
         show_overlap();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> AtoB (Grid) Overlap Reduction" ) )
      {
         last_menu = pos;
         show_grid_overlap();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> Hydrodynamic Calculations" ) )
      {
         last_menu = pos;
         show_hydro();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> Hydrodynamic Calculations Zeno" ) )
      {
         last_menu = pos;
         show_zeno_options();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> Miscellaneous Options" ) )
      {
         last_menu = pos;
         show_misc();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> Bead Model Output" ) )
      {
         last_menu = pos;
         show_bead_output();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> Grid Functions (AtoB)" ) )
      {
         last_menu = pos;
         show_grid();
      }
      pos++;
      if ( res == us_tr( "SOMO Options -> SAXS/SANS Options" ) )
      {
         last_menu = pos;
         show_saxs_options();
      }
      pos++;
      if ( res == us_tr( "MD Options -> DMD Options" ) )
      {
         last_menu = pos;
         show_dmd_options();
      }
      pos++;
      if ( res == us_tr( "MD Options -> Browflex Options" ) )
      {
         last_menu = pos;
         show_bd_options();
      }
      pos++;
      if ( res == us_tr( "MD Options -> Anaflex Options" ) )
      {
         last_menu = pos;
         show_anaflex_options();
      }
      pos++;
      if ( res == us_tr( "PDB Options -> Parsing" ) )
      {
         last_menu = pos;
         pdb_parsing();
      }
      pos++;
      if ( res == us_tr( "PDB Options -> Visualization" ) )
      {
         last_menu = pos;
         pdb_visualization();
      }
      pos++;
      if ( res == us_tr( "Configuration -> Load Configuration" ) )
      {
         last_menu = pos;
         load_config();
      }
      pos++;
      if ( res == us_tr( "Configuration -> Save Current Configuration" ) )
      {
         last_menu = pos;
         write_config();
      }
      pos++;
      if ( res == us_tr( "Configuration -> Reset to Default Configuration" ) )
      {
         last_menu = pos;
         reset();
      }
      pos++;
      if ( res == us_tr( "Configuration -> Advanced Configuration" ) )
      {
         last_menu = pos;
         show_advanced_config();
      }
      pos++;
      if ( res == us_tr( "Configuration -> System Configuration" ) )
      {
         last_menu = pos;
         run_us_config();
      }
      pos++;
      // if ( res == us_tr( "Configuration -> Administrator" ) )
      // {
      //    last_menu = pos;
      //    run_us_admin();
      // }
   }
}

#define TSO QTextStream(stdout)

void US_Hydrodyn::save_params_force_results_name( save_info & save ) {
   QStringList qsl;
   for ( auto field : save.field ) {
      qsl << field;
   }

   US_Vector::printvector( "save_info field", save.field );

   if ( qsl.filter( "results.name" ).size() ) {
      TSO << __func__ << ": found results.name, not inserted\n";
      return;
   }

   save.field.insert( save.field.begin(), "results.name" );
}
