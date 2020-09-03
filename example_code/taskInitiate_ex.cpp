// The QCOR implementation is inconsistent with the QCOR spec in that:
// - the arguments are different
//
// - 

// Call taskInitiate, kick off optimization of the given
// ObjectiveFunction, asynchronous call
  auto handle = qcor::taskInitiate(objective, optimizer, false);

  // Go do other work...

  // Query results when ready.
  auto results = qcor::sync(handle);
