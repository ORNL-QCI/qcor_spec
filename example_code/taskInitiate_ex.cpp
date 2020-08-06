// The code below from the QCOR implementation is inconsistent with the QCOR spec in that:
// - the arguments are different
// - spec says
//     Handle qcore::taskInitiate(Observable *observable, Kernel *kernel, Objective *objective, Optimizer *optimizer)
// - implementation has observable as an argument to createObjectiveFunction

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
