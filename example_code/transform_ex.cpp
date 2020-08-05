  using namespace qcor;
  auto fermi_H = adag(0) * a(1) + adag(1) * a(0);

  //default for transform type is transf = "jw" for the Jordan-Wigner
  auto pauli_H = transform(fermi_H);

  //the below example does an implied qcor::transform on fermi_H and qcor::adag, qcor::a observables to allow algebra between mixed observables
  auto mixed_sum = fermi_H + pauli_H;

  auto mixed_prod = adag(0) * X(1) + X(0) * a(1);

  auto H = X(0) + X(0) * a(1) + adag(1) * X(0) + X(0) * adag(0) * adag(1);
