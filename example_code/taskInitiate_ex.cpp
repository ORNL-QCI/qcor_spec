// The QCOR implementation is inconsistent with the QCOR spec in that:
// - the arguments are different for createObjectiveFunction
//
// - 
__qpu__ void ansatz(qreg q, double theta) {
  X(q[0]);
  Ry(q[1], theta);
  CX(q[1], q[0]);
}

 // Allocate 2 qubits
  auto q = qalloc(2);

  // Create the Deuteron Hamiltonian
  auto H = 5.907 - 2.1433 * X(0) * X(1) - 2.1433 * Y(0) * Y(1) + .21829 * Z(0) -
           6.125 * Z(1);

  // Create the ObjectiveFunction
  auto objective = qcor::createObjectiveFunction(ansatz, H, q, 1);

  // Create the Optimizer
  auto optimizer = qcor::createOptimizer("nlopt");

// Call taskInitiate, kick off optimization of the given
// ObjectiveFunction, asynchronous call
  auto handle = qcor::taskInitiate(objective, optimizer, false);

  // Go do other work...

  // Query results when ready.
  auto results = qcor::sync(handle);
