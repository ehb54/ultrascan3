#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QTextStream>
// #include <Q3ValueList>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os,
                                            const QString &str) {
  return os << qPrintable(str);
}

static unsigned int sgp_creates;
static unsigned int sgp_deletes;

// ---------------------- parameters --------------------------

map<QString, double> sgp_params;

QString sgp_node::validate_params() {
  // validate required parameters:

  QStringList params;
  // will shortening these strings make the map read any quicker?
  params << "distancequantum"
         << "distancemin"
         << "distancemax"
         << "radiusmin"
         << "radiusmax"
         << "branchmax";

  QString qs;
  for (unsigned int i = 0; i < (unsigned int)params.size(); i++) {
    if (!sgp_params.count(params[i])) {
      qs += QString("Error: missing required parameter '%1'\n").arg(params[i]);
    }
  }

  if (sgp_params.count("branchmax") && sgp_params["branchmax"] < 1.0) {
    qs += QString(
              "Error: parameter 'branchmax' must be at least 1 (currently set "
              "to %1)'\n")
              .arg(sgp_params["branchmax"]);
  }

  return qs;
}

QString sgp_node::default_params() {
  QStringList params;

#if QT_VERSION < 0x040000
  //   Q3ValueList < double > values;
#else
  QList<double> values;
#endif

  params << "distancequantum";
  values << 2.5;
  params << "distancemin";
  values << 2.0;
  params << "distancemax";
  values << 10.0;
  params << "radiusmin";
  values << 1.0;
  params << "radiusmax";
  values << 5.0;
  params << "branchmax";
  values << 4.0;

  QString qs;

  for (unsigned int i = 0; i < (unsigned int)params.size(); i++) {
    if (!sgp_params.count(params[i])) {
      sgp_params[params[i]] = values[i];
      qs += QString("Setting parameter '%1' to default value of %2\n")
                .arg(params[i])
                .arg(values[i]);
    } else {
      qs += QString(
                "Parameter '%1' has prexisting value %2, NOT set to default "
                "value of %3\n")
                .arg(params[i])
                .arg(sgp_params[params[i]])
                .arg(values[i]);
    }
  }

  return qs;
}

QString sgp_node::list_params() {
  QString qs;
  for (map<QString, double>::iterator it = sgp_params.begin();
       it != sgp_params.end(); it++) {
    qs += QString("Parameter '%1' has a value of %2\n")
              .arg(it->first)
              .arg(it->second);
  }
  return qs;
}

// ---------------------- support routines
// --------------------------------------

void sgp_node::random_normal(double &r1, double &r2, double mean1, double sd1,
                             double mean2, double sd2) {
  double s;
  do {
    r1 = 2 * drand48() - 1;
    r2 = 2 * drand48() - 1;
    s = r1 * r1 + r2 * r2;
  } while (s > 1);
  s = sqrt(-2 * log(s) / s);
  r1 *= s * sd1;
  r2 *= s * sd2;
  r1 += mean1;
  r2 += mean2;
}

point sgp_node::cross(point p1, point p2)  // p1 cross p2
{
  point result;
  result.axis[0] = p1.axis[1] * p2.axis[2] - p1.axis[2] * p2.axis[1];
  result.axis[1] = p1.axis[2] * p2.axis[0] - p1.axis[0] * p2.axis[2];
  result.axis[2] = p1.axis[0] * p2.axis[1] - p1.axis[1] * p2.axis[0];
  return result;
}

float sgp_node::dot(point p1, point p2)  // p1 dot p2
{
  return p1.axis[0] * p2.axis[0] + p1.axis[1] * p2.axis[1] +
         p1.axis[2] * p2.axis[2];
}

point sgp_node::norm(point p1) {
  point result = p1;
  float divisor =
      sqrt(result.axis[0] * result.axis[0] + result.axis[1] * result.axis[1] +
           result.axis[2] * result.axis[2]);
  result.axis[0] /= divisor;
  result.axis[1] /= divisor;
  result.axis[2] /= divisor;
  return result;
}

point sgp_node::plus(point p1, point p2) {
  point result;
  result.axis[0] = p1.axis[0] + p2.axis[0];
  result.axis[1] = p1.axis[1] + p2.axis[1];
  result.axis[2] = p1.axis[2] + p2.axis[2];
  return result;
}

point sgp_node::scale(point p, float m) {
  for (unsigned int i = 0; i < 3; i++) {
    p.axis[i] *= m;
  }
  return p;
}

// ---------------------- constructor, basic
// manipulation--------------------------

sgp_node::sgp_node() {
  sgp_creates++;
  children.clear();
  parent = (sgp_node *)0;
  normal.axis[0] = 0.0;
  normal.axis[1] = 0.0;
  normal.axis[2] = 0.0;
  distance = 0;
  radius = 0;
  is_dead = false;
  fitness_ok = false;
}

sgp_node::sgp_node(point normal, unsigned int distance, unsigned int radius) {
  sgp_creates++;
  children.clear();
  parent = (sgp_node *)0;
  this->normal = normal;
  this->distance = distance;
  this->radius = radius;
  is_dead = false;
  fitness_ok = false;
}

sgp_node::~sgp_node() {
  sgp_deletes++;

  // cout << QString( "in deconstructor: has parent %1, children size %2,
  // distance %3, radius %4\n" )
  //      .arg( parent ? "yes" : "no" )
  // .arg( children.size() )
  // .arg( distance )
  // .arg( radius );

  list<sgp_node *>::iterator it = children.begin();

  while ((it = children.begin()) != children.end()) {
    // cout << "deleting children:\n";
    delete *it;
    // cout << "deleting children iterator increment:\n";
  }

  if (parent) {
    // cout << "removing from parent:\n";
    parent->children.remove(this);
  }
}

QString sgp_node::contents(bool include_children) {
  QString qs;

  qs += QString("sgp: depth %1 distance %2 radius %3 normal [%4,%5,%6]\n")
            .arg(depth())
            .arg(distance)
            .arg(radius)
            .arg(normal.axis[0])
            .arg(normal.axis[1])
            .arg(normal.axis[2]);

  if (include_children) {
    for (list<sgp_node *>::iterator it = children.begin(); it != children.end();
         it++) {
      qs += (*it)->contents();
    }
  }

  return qs;
}

QString sgp_node::contents_by_pos() {
  QString qs;

  QString spaces = "                                                  ";

  for (unsigned int i = 0; i < size(); i++) {
    qs +=
        QString(
            "sgp: pos %1 %2 depth %3 distance %4 radius %5 normal [%6,%7,%8]\n")
            .arg(i)
            .arg(spaces.left(ref(i)->depth()))
            .arg(ref(i)->depth())
            .arg(ref(i)->distance)
            .arg(ref(i)->radius)
            .arg(ref(i)->normal.axis[0])
            .arg(ref(i)->normal.axis[1])
            .arg(ref(i)->normal.axis[2]);
  }

  return qs;
}

unsigned int sgp_node::size() {
  unsigned int i = 1;

  for (list<sgp_node *>::iterator it = children.begin(); it != children.end();
       it++) {
    i += (*it)->size();
  }

  return i;
}

unsigned int sgp_node::depth() {
  unsigned int i = 0;

  sgp_node *tmp_sgp = parent;

  while (tmp_sgp) {
    i++;
    tmp_sgp = tmp_sgp->parent;
  }

  return i;
}

sgp_node *sgp_node::root() {
  if (!parent) {
    return this;
  }
  return parent->root();
}

point sgp_node::checksum() {
  point p = scale(normal, distance + radius);
  for (list<sgp_node *>::iterator it = children.begin(); it != children.end();
       it++) {
    point p0 = (*it)->checksum();
    p.axis[0] += p0.axis[0];
    p.axis[1] += p0.axis[1];
    p.axis[2] += p0.axis[2];
  }
  return p;
}

sgp_node *sgp_node::ref(unsigned int pos) {
  // cout << QString( "ref called pos %1 size %2\n" ).arg( pos ).arg( size() );
  if (pos >= size()) {
    return (sgp_node *)0;
  }

  if (!pos) {
    // cout << QString( "return this\n" );
    return this;
  }

  pos--;

  for (list<sgp_node *>::iterator it = children.begin(); it != children.end();
       it++) {
    // cout << QString( "going through children, pos now %1\n" ).arg( pos );
    if ((*it)->size() > pos) {
      return (*it)->ref(pos);
    }
    pos -= (*it)->size();
  }

  // shouldn't get here
  cout << QString("Internal error: sgp_node::returning 0 at end\n");
  return (sgp_node *)0;
}

sgp_node *sgp_node::copy(sgp_node *node) {
  sgp_node *tmp_node = new sgp_node(node->normal, node->distance, node->radius);
  for (list<sgp_node *>::iterator it = node->children.begin();
       it != node->children.end(); it++) {
    sgp_node *tmp2_node = copy(*it);
    tmp2_node->parent = tmp_node;
    tmp_node->children.push_back(tmp2_node);
  }
  return tmp_node;
}

bool sgp_node::insert_copy(unsigned int pos, sgp_node *node) {
  if (pos >= size()) {
    return false;
  }

  if (!pos) {
    sgp_node *tmp_node = copy(node);
    tmp_node->parent = this;
    children.push_back(tmp_node);
    return true;
  } else {
    return ref(pos)->insert_copy(0, node);
  }
}

// -------------------------- model output -------------------------------

point sgp_node::get_coordinate() {
  point p1;
  if (!parent ||
      (parent->normal.axis[0] == 0.0 && parent->normal.axis[1] == 0.0 &&
       parent->normal.axis[2] == 0.0)) {
    p1.axis[0] = 1.0;
    p1.axis[1] = 0.0;
    p1.axis[2] = 0.0;
  } else {
    // if parallel, go up towards root
    sgp_node *use = parent;
    do {
      p1 = cross(use->normal, normal);
      use = use->parent;
    } while (use && p1.axis[0] == 0.0 && p1.axis[1] == 0.0 &&
             p1.axis[2] == 0.0);

    // if parallel to root's normal:

    if (p1.axis[0] == 0.0 && p1.axis[1] == 0.0 && p1.axis[2] == 0.0) {
      // just set to normal direction when parallel
      point p0;
      p0.axis[0] = 1.0;
      p0.axis[1] = 0.0;
      p0.axis[2] = 0.0;
      p1 = cross(p0, normal);
      if (p1.axis[0] == 0.0 && p1.axis[1] == 0.0 && p1.axis[2] == 0.0) {
        // normal must be 1,0,0, just keep it
        p1 = normal;
      }
    }
  }

  // point ptest = norm( p1 );

  p1 = scale(norm(p1), (float)distance);
  if (parent) {
    p1 = plus(p1, parent->get_coordinate());
  }  // else: rooted at 0,0,0
  return p1;
}

vector<sgp_sphere> sgp_node::sgp_spheres() {
  vector<sgp_sphere> result;

  for (unsigned int i = 0; i < size(); i++) {
    sgp_sphere tmp_sphere;
    tmp_sphere.radius = ref(i)->radius;
    tmp_sphere.coordinate = ref(i)->get_coordinate();

    result.push_back(tmp_sphere);
  }

  return result;
}

QString sgp_node::qs_bead_model() {
  vector<sgp_sphere> spheres = sgp_spheres();

  QString qs;

  qs += QString("%1 %2\n").arg(spheres.size()).arg(.7);

  for (unsigned int i = 0; i < spheres.size(); i++) {
    qs +=
        QString("%1 %2 %3 %4  20 8 Unk 10\n")
            .arg(spheres[i].coordinate.axis[0] * sgp_params["distancequantum"])
            .arg(spheres[i].coordinate.axis[1] * sgp_params["distancequantum"])
            .arg(spheres[i].coordinate.axis[2] * sgp_params["distancequantum"])
            .arg(spheres[i].radius * sgp_params["distancequantum"]);
  }
  return qs;
}

vector<PDB_atom> sgp_node::bead_model() {
  vector<sgp_sphere> spheres = sgp_spheres();

  vector<PDB_atom> result;

  for (unsigned int i = 0; i < spheres.size(); i++) {
    PDB_atom tmp_atom;
    tmp_atom.coordinate.axis[0] =
        spheres[i].coordinate.axis[0] * sgp_params["distancequantum"];
    tmp_atom.coordinate.axis[1] =
        spheres[i].coordinate.axis[1] * sgp_params["distancequantum"];
    tmp_atom.coordinate.axis[2] =
        spheres[i].coordinate.axis[2] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[0] =
        spheres[i].coordinate.axis[0] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[1] =
        spheres[i].coordinate.axis[1] * sgp_params["distancequantum"];
    tmp_atom.bead_coordinate.axis[2] =
        spheres[i].coordinate.axis[2] * sgp_params["distancequantum"];
    tmp_atom.bead_computed_radius =
        spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.bead_actual_radius =
        spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.radius = spheres[i].radius * sgp_params["distancequantum"];
    tmp_atom.bead_mw = 20;
    tmp_atom.mw = 20;
    tmp_atom.bead_ref_volume = 0;
    tmp_atom.bead_color = 1;
    tmp_atom.serial = i + 1;
    tmp_atom.exposed_code = 1;
    tmp_atom.all_beads.clear();
    tmp_atom.name = "SGP";
    tmp_atom.resName = "SGP";
    tmp_atom.iCode = "";
    tmp_atom.chainID = "";
    tmp_atom.chain = 1;
    tmp_atom.active = 1;
    tmp_atom.normalized_ot_is_valid = false;

    result.push_back(tmp_atom);
  }

  return result;
}

// -------------------------- creation routines & genetic operators
// --------------------------

// create a random node of size "size"

sgp_node *sgp_node::random(unsigned int size) {
  if (!size) {
    return (sgp_node *)0;
  }

  // create base node

  point n;

  // we are going do be making lots of these, so lets make sure we don't have a
  // zero vector:

  point nn;
  do {
    n.axis[0] = drand48() - 0.5;
    n.axis[1] = drand48() - 0.5;
    n.axis[2] = drand48() - 0.5;
    nn = norm(n);
  } while ((fabs(n.axis[0]) < 1e-5 && fabs(n.axis[1]) < 1e-5 &&
            fabs(n.axis[2]) < 1e-5) ||
           us_isnan(nn.axis[0]) || us_isnan(nn.axis[1]) ||
           us_isnan(nn.axis[2]));

  unsigned int dist = (unsigned int)(sgp_params["distancemin"] +
                                     drand48() * (sgp_params["distancemax"] -
                                                  sgp_params["distancemin"]));
  unsigned int rad = (unsigned int)(sgp_params["radiusmin"] +
                                    drand48() * (sgp_params["radiusmax"] -
                                                 sgp_params["radiusmin"]));

  // we could remove overlap by forcing distance to be adjacent, but we want
  // arbitrary electron densities for now

  sgp_node *new_sgp = new sgp_node(n, dist, rad);

  for (unsigned int i = 1; i < size; i++) {
    unsigned int pos;
    if (sgp_params["branchmax"] > 1) {
      do {
        pos = (unsigned int)(drand48() * i);
      } while (new_sgp->ref(pos)->children.size() >= sgp_params["branchmax"]);
    } else {
      pos = i - 1;
    }

    // cout << QString( "random inserting at position %1\n" ).arg( pos );
    sgp_node *tmp_sgp = random(1);
    tmp_sgp->parent = new_sgp->ref(pos);
    new_sgp->ref(pos)->children.push_back(tmp_sgp);
  }
  return new_sgp;
}

bool sgp_node::mutate(unsigned int pos) {
  if (size() <= pos) {
    return false;
  }

  sgp_node *node = ref(pos);

  double selector = drand48();

  double new_distance;
  double new_radius;

  random_normal(new_distance, new_radius, (double)distance,
                sgp_params["distancesd"], (double)radius,
                sgp_params["radiussd"]);

  if (selector < .667) {
    if (new_distance < sgp_params["distancemin"]) {
      new_distance = sgp_params["distancemin"];
    }
    if (new_distance > sgp_params["distancemax"]) {
      new_distance = sgp_params["distancemax"];
    }

    node->distance = (unsigned int)new_distance;
  }

  if (selector > .334) {
    if (new_radius < sgp_params["radiusmin"]) {
      new_radius = sgp_params["radiusmin"];
    }
    if (new_radius > sgp_params["radiusmax"]) {
      new_radius = sgp_params["radiusmax"];
    }

    node->radius = (unsigned int)new_radius;
  }

  return true;
}

bool sgp_node::crossover(sgp_node *&result, sgp_node *node1, sgp_node *node2) {
  unsigned int pos1 = (unsigned int)(node1->size() * drand48());
  unsigned int pos2 = (unsigned int)(node2->size() * drand48());

  if (pos1) {
    result = copy(node1);
    sgp_node *pref = result->ref(pos1)->parent;
    delete result->ref(pos1);
    pref->insert_copy(0, node2->ref(pos2));
  } else {
    result = copy(node2->ref(pos2));
  }
  return true;
}

// --------------------------- test ------------------------------------

void sgp_node::check_normal(QString qs) {
  for (unsigned int i = 0; i < size(); i++) {
    if (ref(i)->normal.axis[0] == 0.0 && ref(i)->normal.axis[1] == 0.0 &&
        ref(i)->normal.axis[2] == 0.0) {
      cout << QString("Error %1: found zero zero zero normal at pos %2\n")
                  .arg(qs)
                  .arg(i);
    }
  }
}

void sgp_node::test() {
  cout << default_params();

  if (!validate_params().isEmpty()) {
    cout << validate_params();
    return;
  }

  sgp_creates = 0;
  sgp_deletes = 0;

  srand48((long int)QTime::currentTime().msec());

  point n;

  n.axis[0] = drand48();
  n.axis[1] = drand48();
  n.axis[2] = drand48();

  sgp_node *our_sgp = new sgp_node(n, 1, 2);

  cout << QString("current creates %1 deletes %2\n")
              .arg(sgp_creates)
              .arg(sgp_deletes);

  cout << "Current size(): " << our_sgp->size() << endl;
  cout << "Initial node:\n" << our_sgp->contents();
  vector<sgp_sphere> spheres = our_sgp->sgp_spheres();

  n.axis[0] = drand48();
  n.axis[1] = drand48();
  n.axis[2] = drand48();

  sgp_node *tmp_sgp = new sgp_node(n, 2, 3);

  our_sgp->insert_copy(0, tmp_sgp);

  delete tmp_sgp;

  cout << QString("current creates %1 deletes %2\n")
              .arg(sgp_creates)
              .arg(sgp_deletes);
  cout << "Current size(): " << our_sgp->size() << endl;
  cout << "Added one node:\n" << our_sgp->contents();

  n.axis[0] = drand48();
  n.axis[1] = drand48();
  n.axis[2] = drand48();

  tmp_sgp = new sgp_node(n, 3, 4);
  our_sgp->insert_copy(0, tmp_sgp);
  delete tmp_sgp;

  cout << QString("current creates %1 deletes %2\n")
              .arg(sgp_creates)
              .arg(sgp_deletes);
  cout << "Current size(): " << our_sgp->size() << endl;
  cout << "Added two at top level:\n" << our_sgp->contents();

  cout << "List by pos:\n";
  for (unsigned int i = 0; i < our_sgp->size(); i++) {
    cout << i << " : "
         << (our_sgp->ref(i) ? our_sgp->ref(i)->contents(false) : "null pos\n");
  }

  // ok, lets make a bunch of random nodes

  cout << "now delete\n";
  delete our_sgp;

  for (unsigned int i = 0; i < 10; i++) {
    cout << QString("current creates %1 deletes %2\n")
                .arg(sgp_creates)
                .arg(sgp_deletes);
    cout << QString("random try %1 size %2\n").arg(i).arg(i * 10 + 10);
    our_sgp = random(i * 10 + 10);
    cout << "Current size(): " << our_sgp->size() << endl;
    cout << "contents:\n" << our_sgp->contents();

    cout << "List by pos:\n";
    for (unsigned int k = 0; k < our_sgp->size(); k++) {
      cout << k << " : "
           << (our_sgp->ref(k) ? our_sgp->ref(k)->contents(false)
                               : "null pos\n");
    }
    vector<sgp_sphere> spheres = our_sgp->sgp_spheres();
    for (unsigned int j = 0; j < spheres.size(); j++) {
      cout << QString("%1: [%2,%3,%4] r %5\n")
                  .arg(j)
                  .arg(spheres[j].coordinate.axis[0])
                  .arg(spheres[j].coordinate.axis[1])
                  .arg(spheres[j].coordinate.axis[2])
                  .arg(spheres[j].radius);
    }

    cout << "trying to write\n";
    QFile f(QString("sgp_%1.bead_model").arg(i));

    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << our_sgp->qs_bead_model();
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
    }

    cout << "now delete\n";
    delete our_sgp;
  }

  // ---------- test mutate -------------

  our_sgp = random(10);
  sgp_node *our_sgp2;
  for (unsigned int i = 0; i < 10; i++) {
    our_sgp2 = copy(our_sgp);
    our_sgp2->mutate((unsigned int)(our_sgp2->size() * drand48()));

    QFile f(QString("sgp_%1_mutate.bead_model").arg(i));

    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << our_sgp2->qs_bead_model();
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
    }
    delete our_sgp2;
  }
  delete our_sgp;

  // ---------- test crossover -------------
  our_sgp = random(5);
  our_sgp2 = random(5);

  for (unsigned int i = 0; i < 10; i++) {
    sgp_node *our_cross;
    crossover(our_cross, our_sgp, our_sgp2);

    QFile f(QString("sgp_%1_cross.bead_model").arg(i));

    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << our_cross->qs_bead_model();
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
    }
    delete our_cross;
  }

  delete our_sgp;
  delete our_sgp2;

  cout << QString("current creates %1 deletes %2\n")
              .arg(sgp_creates)
              .arg(sgp_deletes);
}

QString sgp_node::usage() {
  return QString("current creates %1 deletes %2 open %3\n")
      .arg(sgp_creates)
      .arg(sgp_deletes)
      .arg(sgp_creates - sgp_deletes);
}
