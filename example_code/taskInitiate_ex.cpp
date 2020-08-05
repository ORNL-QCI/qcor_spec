// Call taskInitiate, kick off optimization of the given
// functor dependent on the ObjectiveFunction, async call
  auto handle = qcor::taskInitiate(
      objective, optimizer,
      [&](const std::vector<double> x, std::vector<double> &dx) {
        return (*objective)(q, x[0]);
      },
      1);

  // Go do other work...

  // Query results when ready.
  auto results = qcor::sync(handle);
