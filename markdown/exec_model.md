---
title: "Execution Model"
date: 2019-11-29T15:26:15Z
draft: false
weight: 15
---

The QCOR execution model considers a hybrid quantum-classical compute system that enables execution of quantum kernels on available quantum coprocessors in a quantum-hardware agnostic fashion. The host-side, classical execution model should simply be the execution model of the language being extended (e.g. C++, Python, etc.). 

QCOR treats the quantum coprocessors as an abstract machine that loads and executes general host-specified quantum instructions and persists measurement results (measured classical bits) to a quantum-processor-local discrete memory space, with synchronization of that memory space with the host-side handled implicitly by language extension implementations. Each specified quantum instruction for execution carries with it information about its unique instruction identifier, the qubit to operate upon (the index of the qubit in the infinitely-sized, assumed global qubit register, see [Memory Model](memory.md)), and any required or extra metadata about the operation. Quantum instructions loaded for execution can be single instructions or a composite of single instructions, thereby enabling one to submit, load, and execute entire quantum circuits or programs. 

This latter point enables the QCOR execution model to retain a multi-modal characteristic. The execution model should enable both near-term, loosely-coupled (remotely-hosted) batch circuit execution models (NISQ mode execution), as well as future, tightly integrated CPU-QPU architectures with fast-feedback (FTQC mode execution, fault-tolerant quantum computing). FTQC mode execution streams quantum instruction executions, i.e. as the program is running, invoked quantum instructions will affect the qubit register as soon as they are encountered. NISQ mode execution should instead queue encountered quantum instructions and flush the queue upon exiting an entry-point quantum kernel. 

Language implementations should enable both modes of execution, and this implies additional programming capabilities that may exist in one mode over the other. For example, FTQC mode execution should enable programmers to leverage the classical language control flow structures enabling fast-feedback on qubit measurement results. 

## QCOR Runtime Model

The QCOR runtime handles execution parameters that either query or manipulate the state of QPU settings as part of a hybrid quantum-classical system. Current QPU implementations are limited to one possible target backend, but we expect that future QCOR runtimes would need to support multiple QPU backends each with their own runtime-specific set of parameters. As an example, future runtime systems will likely encapsulate the concept of multi-modal execution where a QPU could be manipulated to run in either NISQ or FTQC mode depending on the desired experiment and application. 

The current QCOR specification does not require runtime features to be implemented, but we anticipate that specific features may be useful to be implemented as part of future QPUs. 

* `GetNumQPUs` - specify the set of available QPUs for execution of a QCOR program 
* `{Set/Get}TargetQreqs` - specify a set of qubits for allocation and execution of a quantum program
* `{Set/Get}TargetQPU` - specify or query a default QPU  

### Thread Safety and Multiple Host Threads

QCOR API calls including `qalloc`, `createObjectiveFunction`, `createOptimizer`, `optimize`, and quantum kernel invocation are thread-safe. Thus, it is safe for multiple host threads to call QCOR API calls concurrently. However, the behavior of the QCOR API in terms of multi-threaded execution is implementation-specific. Specifically, one implementation may run multiple QCOR execution instances in parallel to fully utilize the underlying hardware, whereas another implementation may just serialize these instances.

The following examples illustrate multi-threading with C++ thread (std::thread). In Example 1, each thread performs `qalloc`, quantum kernel invocation, and then prints the results. Since these API calls are thread-safe, two instances of kernel simulation can be safely executed. Similarly, in Example 2, each thread performs the VQE workflow (`qalloc`, `createObjectiveFunction`, `createOptimizer`, `optimize`), which can also be done safely. Example 3 is a variant of Example 2, where two `optimize` calls are asynchronously invoked with`std::async` and the completion of each `async` can be waited by calling `std::future::get`.

#### Example 1: two threads perform `qalloc` and kernel invocation concurrently

```cpp
// the bell kernel
_qpu__ void bell(qreg q) { … }

void run_bell() {
  // Create two qubit registers, each size 2
  auto q = qalloc(2);
  // Run the quantum kernel
  bell(q);
  // dump the results
  q.print();
}

int main(int argc, char **argv) {
    // two threads execution
    std::thread t0(run_bell);
    std::thread t1(run_bell);
    t0.join();
    t1.join();
}
```

#### Example 2: two threads peform the VQE workflow concurrently

```cpp
__qpu__ void ansatz(qreg q, double theta) {
  X(q[0]);
  Ry(q[1], theta);
  CX(q[1], q[0]);
}

void run_vqe() {
  // Allocate 2 qubits
  auto q = qalloc(2);

  // Programmer needs to set
  // the number of variational params
  auto n_variational_params = 1;

  // Create the Deuteron Hamiltonian
  auto H = 5.907 - 2.1433 * X(0) * X(1) - 2.1433 * Y(0) * Y(1) + .21829 * Z(0) -
           6.125 * Z(1);

  // Create the ObjectiveFunction, here we want to run VQE
  // need to provide ansatz, Operator, and qreg
  auto objective = createObjectiveFunction(ansatz, H, q, n_variational_params,
                                           {{"gradient-strategy", "central"}, {"step", 1e-3}});

  // Create the Optimizer.
  auto optimizer = createOptimizer("nlopt", {{"nlopt-optimizer", "l-bfgs"}});

  // Launch the Optimization Task with taskInitiate

  auto [opt_val, opt_params] = optimizer->optimize(objective);
  printf("vqe-energy = %f\n", opt_val);
}

int main(int argc, char** argv) {
    std::thread t0(run_vqe);
    std::thread t1(run_vqe);
    t0.join();
    t1.join();
}
```

#### Example 3: Asynchonous execution of `qcor::optimizer` using `std::async` and `std::future`

```cpp
__qpu__ void ansatz(qreg q, double theta) {
  X(q[0]);
  Ry(q[1], theta);
  CX(q[1], q[0]);
}

int main(int argc, char **argv) {
  // Allocate 2 qubits
  auto q1 = qalloc(2);
  auto q2 = qalloc(2);

  // Programmer needs to set
  // the number of variational params
  auto n_variational_params1 = 1;
  auto n_variational_params2 = 1;

  // Create the Deuteron Hamiltonian
  auto H1 = 5.907 - 2.1433 * X(0) * X(1) - 2.1433 * Y(0) * Y(1) + .21829 * Z(0) -
           6.125 * Z(1);
  auto H2 = 5.907 - 2.1433 * X(0) * X(1) - 2.1433 * Y(0) * Y(1) + .21829 * Z(0) -
           6.125 * Z(1);

  // Create the ObjectiveFunction, here we want to run VQE
  // need to provide ansatz, Operator, and qreg
  auto objective1 = createObjectiveFunction(ansatz, H1, q1, n_variational_params1,
                                            {{"gradient-strategy", "central"}, {"step", 1e-3}});
  auto objective2 = createObjectiveFunction(ansatz, H2, q2, n_variational_params2,
                                            {{"gradient-strategy", "central"}, {"step", 1e-3}});

  // Create the Optimizer.
  auto optimizer1 = createOptimizer("nlopt", {{"nlopt-optimizer", "l-bfgs"}});
  auto optimizer2 = createOptimizer("nlopt", {{"nlopt-optimizer", "l-bfgs"}});

  // Launch the two optimizations asynchronously
  auto handle1 = std::async(std::launch::async, [=]() -> std::pair<double, std::vector<double>> { return optimizer1->optimize(objective1); });
  auto handle2 = std::async(std::launch::async, [=]() -> std::pair<double, std::vector<double>> { return optimizer2->optimize(objective2); });

  // Go do other work...

  // Query results when ready.
  auto [opt_val1, opt_params1] = handle1.get();
  auto [opt_val2, opt_params2] = handle2.get();

  printf("vqe-energy1 = %f\n", opt_val1);
  printf("vqe-energy2 = %f\n", opt_val2);
}

```

## Execution Model Examples

Here are two examples showing the differences in programming for NISQ and FTQC mode execution (in a C++ QCOR language extension). First we show FTQC mode execution, where one can clearly see the ability to stream quantum instruction invocations, followed by conditional statements on qubit measurement results. 
```cpp
__qpu__ void PrepareStateUsingRUS(qreg q, int maxIter) {
  // Note: target = q[0], aux = q[1]
  auto q = q.head();
  auto r = q.tail();
  H(r);
  // We limit the max number of RUS iterations.
  for (int i = 0; i < maxIter; ++i) {
    Tdg(r);
    X::ctrl(q, r);
    T(r);

    // In order to measure in the PauliX basis, changes the basis.
    H(r);
    if (!Measure(r)) {
      // Success (until (outcome == Zero))
      std::cout << "Success after " << i + 1 << " iterations.\n";
      break;
    } 
    else {
      // Measure 1: |1> state
      // Fix up: Bring the auxiliary and target qubits back to |+> state.
      X(r);
      H(r);
      X(q);
      H(q);
    }
  }
}
```

Next, we show the construction of a QAOA ansatz on a general cost Hamiltonian. We still get the usual control flow (`for` loops, etc.), but the circuit is in a sense constructed by this quantum kernel before ultimate execution on the loosely-coupled quantum processor. The stream of instructions is implicitly flushed (executed) upon exiting this entry-point quantum kernel. 
```cpp
__qpu__ void qaoa_ansatz(qreg q, int n_steps, std::vector<double> gamma,
                         std::vector<double> beta, Operator cost_ham) {

  // Local Declarations
  auto nQubits = q.size();
  int gamma_counter = 0;
  int beta_counter = 0;

  // Start of in the uniform superposition
  H(q);

  // Get all non-identity hamiltonian terms
  // for the following exp(H_i) trotterization
  auto cost_terms = cost_ham.getNonIdentitySubTerms();

  // Loop over qaoa steps
  for (int step = 0; step < n_steps; step++) {

    // Loop over cost hamiltonian terms
    for (int i = 0; i < cost_terms.size(); i++) {

      auto cost_term = cost_terms[i];
      auto m_gamma = gamma[gamma_counter];

      // trotterize
      exp_i_theta(q, m_gamma, cost_term);

      gamma_counter++;
    }

    // Add the reference hamiltonian term
    for (int i = 0; i < nQubits; i++) {
      auto ref_ham_term = X(i);
      auto m_beta = beta[beta_counter];
      exp_i_theta(q, m_beta, ref_ham_term);
      beta_counter++;
    }
  }
}
```
