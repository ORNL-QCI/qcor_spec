// The QCOR implementation code below in inconsistent with the spec. The spec creates the observable in two steps:
//   Observable qcor::createPauli(std::map<int,string> sitemap, std::complex coef, string paulistr)
//
//   Observable* qcor::createObservable (Observable &obsvOp, FieldOperator &fieldOp)
//
// Create the Dueteron Hamiltonian (Observable)
auto H = qcor::createObservable(
      "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1");

auto H = 5.907 - 2.1433 * qcor::X(0) * qcor::X(1) -
           2.1433 * qcor::Y(0) * qcor::Y(1) + .21829 * qcor::Z(0) -
           6.125 * qcor::Z(1);

using namespace qcor;
auto H = -0.5 * adag(0) * a(0) + 4.0 * adag(0) * a(0) * adag(1) * a(1) - 
            1.0 * adag(0) * a(1) - 1.0 * adag(0) * a(2) -1.0 * adag(1) * a(0) - 
            0.5 * adag(1) * a(1) + 4.0 * adag(1) * a(1) * adag(2) * a(2) - 
            1.0 * adag(1) * a(2) - 1.0 * adag(1) * a(2) - 1.0 *adag(2) * a(0) - 
            1.0 *adag(2) * a(1) - 0.5 * adag(2) * a(2) + 4.0 * adag(2) * a(2) * adag(0) * a(0);

auto H = createObservable("fermion", "(-0.5, 0.0) * 0^ 0 +"
                                    "(4.0, 0.0) * 0^ 0 1^ 1 +"
                                    "(-1.0, 0.0) * 0^ 1 +"
                                    "(-1.0, 0.0) * 0^ 2 +"
                                    "(-1.0, 0.0) * 1^ 0 +"
                                    "(-0.5, 0.0) * 1^ 1 +"
                                    "(4.0, 0.0) * 1^ 1 2^ 2 +"
                                    "(-1.0, 0.0) * 1^ 2 +"
                                    "(-1.0, 0.0) * 2^ 0 +"
                                    "(-1.0, 0.0) * 2^ 1 +"
                                    "(-0.5, 0.0) * 2^ 2 +"
                                    "(4.0, 0.0) * 2^ 2 0^ 0" );
