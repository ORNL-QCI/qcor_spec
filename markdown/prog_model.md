---
title: "Programming Model"
date: 2019-11-29T15:26:15Z
draft: false
weight: 15
---

qcor enables the programming of heterogeneous quantum-classical computing tasks, whereby programmers are free to leverage both classical and quantum processors to achieve some hybrid workflow goal. One can think of the types of hybrid quantum-classical programs that qcor enables as sitting on a spectrum with purely classical codes on one end and purely quantum codes on the other (with required classical driver code). Our programming model enables one to program across this spectrum, thereby enabling a language expression mechanism for near-term, noisy intermediate-scale as well as future fault-tolerant quantum tasks. 

qcor requires that specification implementors extend classical programming languages with support for quantum in a single-source manner. This requirement directly promotes familiarity and usability for domain computational scientist, and lowers the learning curve for separate quantum and classical compiler workflows and manual integration to produce quantum-classical executables. This single-source language extension approach enables quantum-classical tasks to be constructed using high level classical language syntax via intrinsic library calls and compiler preprocessor directives. 

### Allocating Quantum Memory

As described in the [Memory Model](memory.md), qcor specifies `qubit` and `qreg` types that serve as a handle on allocated quantum memory. Allocation of quantum memory is achieved through an intrinsic library call named `qalloc` (must be provided by language extension implementation runtime library), which returns an allocated `qreg` instance of the programmer-specified size. Deallocation of quantum resources should occur implicitly when the `qreg` instance goes out of scope. While the `qubit` type is specified to be opaque, the `qreg` type should provide a public API for sub-register extraction, individual `qubit` addressing, slicing, concatenation, and retrieval of `qubit` measurement results. qcor specifies two function overloads for `qalloc()`: (1) the default that takes an `int`-like data type describing the size of the register, and (2) an overload that takes the size and a `bool` indicating that stack-allocated `qubit` instances will be uncomputed manually (`true` indicates that uncomputation will be manual and handled by the programmer). 

Basic `qreg` usage:
```cpp
auto q = qalloc(10);
kernel(q);
auto counts = q.counts();
for (auto [bits, count] : counts) {
    print(bits, ":", count);
}
```
Basic `qreg` operations within kernels:
```cpp
__qpu__ void kernel(qreg q, int n) {
    auto first = q.head();
    auto first_n = q.head(n);
    auto last = q.tail();
    auto last_n = q.tail(n);
    auto extracted = q.extract({3, 14});
    auto addressed = q[13];

    auto ancilla = qalloc(2);
    ...
}
```

### Quantum Kernels

To adhere to the single-source requirement promoted by qcor, quantum code intended for compilation and execution targeting a desired quantum coprocessor should be expressed as typical callables in the source 
language being extended. Examples of this would be standard functions in C++, Python, Julia, etc., closures/lambdas in C++, or structs with `operator()()` defined in C++. Quantum kernels provided as standard functions or lambdas should be able to reference and call previously defined or declared classical functions, classical global variables, and quantum kernels. Quantum kernels provided as lambdas should be able to capture classical variables from parent scope. Quantum kernel lambdas should not be able to capture quantum data (`qreg`). All quantum data must be provided to the quantum kernel via function argument (except for in-kernel, stack-allocated `qreg` instances). 

The quantum kernel function signature can have any structure, however, the return type must be `void`. The body of the callable should contain code expressing the quantum program to be executed on the quantum coprocessor. The function body language is the native language being extended (for classical control flow and classical data allocation), plus the invocation of [Quantum Intrinsic Operations](#q-intr), [common quantum programming patterns](#q-patterns), or [unitary matrix decompositions](#q-unitary). As a native language extension, the qcor programming model specifies that any classical control flow be available within quantum kernel definitions. Programmers should be able to fully use `for` and `while` loops (for example) as expected, as well as `if` statements (`if {} else {}`, etc.), even in the case of conditional sub-circuit execution based on qubit measurement feedback. 

Quantum kernels, as functional callables, should also be able to be passed as parameters to other function-like objects (or other quantum kernels). A typical use case for this would be the development of a generic quantum algorithm that is parameterized on some oracle (which would be expressed as some other quantum kernel). 

Defined quantum kernels should expose a public API that produces related quantum kernels. By related quantum kernels, we mean new quantum kernels that are defined in relation to the original kernel. Methods that must be on the quantum kernel are (here we use static method definitions, but one could also use instance methods (`.`) as well): 
- `kernel::ctrl( qubit*, Args...)`, which takes an array-like of `qubits` (e.g. C++ implementation may use `std::vector<qubit>`) as the control qubits, takes all usual kernel arguments
- `kernel::adjoint(Args...)`, which takes the original kernel arguments but applies the reverse of the kernel, or adjoint
- `kernel::observe(Operator*, Args...)` to take an unmeasured kernel and return the expected value of the given [`Operator`](data_model.md) at the provided kernel arguments.
- `kernel::print(Stream&, Args...)` to print a QASM-like representation of the defined kernel in the native gateset of the target backend. 

Quantum kernels can be invoked from other quantum kernles, enabling a hierarchical representation of complex programs as well as the integration of pre-developed quantum library code (e.g. libraries for quantum Fourier transformations, Hadamard and Swap tests, etc.). The specification denotes a quantum kernel that is called from host classical code as an **entry-point quantum kernel**, in order to differentiate itself from quantum kernels invoked from other quantum kernels. All entry-point quantum kernels must take at least one `qreg` or `qubit` instance in order to ensure that downstream classical code can retrieve quantum execution results. 

#### <a id="q-intr"></a>Quantum Intrinsic Operations and Expressions

Quantum intrinsic operations are quantum instructions provided by the language extension implementation that can only be invoked from within quantum kernels (they are not runtime library calls, instead they are native expressions in the language being extended). These should typically be quantum gate calls that are native to the language extension and affect execution of the specific instruction on the targeted quantum coprocessor. The implementation is free to define a list of default available quantum instructions, but typically one should provide instruction calls for `Hadamard`, `X`, `Y`, `Z`, `Rx`, `Ry`, `Rz`, `controlled-X`, `controlled-Y`, `controlled-Z`, `controlled-Hadamard`, `phase`, `T`, `Swap`, and `Reset` operations. 

All single-qubit operations should also provide the usual `kernel::ctrl(...)` operation (e.g. enable operations like `X::ctrl({q,r}, s) == Toffoli(q,r,s)`). All single qubit intrinsic operations should broadcast over provided `qreg` instances, i.e. given a `qreg` instead of a `qubit`, quantum operations should apply the operation to all qubits in the `qreg`. Two qubit operations should also broadcast according to the following rules: `OP(qreg_0, qreg_1) == OP(qreg_0[i], qreg_1[i]) for all i in qreg_0.size()` (`qreg` must have same size).

Here is a basic example in C++ demonstrating kernel definition using quantum intrinsic operations, `::ctrl`, `Measure` broadcast on all qubits in `q`, and classical control flow. 
```cpp
__qpu__ void ghz(qreg q) {                   // NOTES:
    auto first_qubit = q.head();             // qreg API
    H(first_qubit);                          // Quantum Intrinsic Operation
    for (auto i : range(q.size()-1)) {       // Can use C++ ctrl flow
        X::ctrl(q[i], q[i+1]);               // Single qubit gates have ::ctrl
    }
    Measure(q);                              // qreg operation broadcasting
}
int main() {
    auto q = qalloc(10);                     // Allocate 10 qubits, standard library qalloc
    ghz(q);                                  // Invoke the quantum kernel
    for (auto [bits, count] : q.counts()) {  // Read out the results
        print(bits, ":", count);
    }
    // qreg q out of scope, deallocation occurs
}
```


##### <a id="q-patterns"></a> Compute-Action-Uncompute
The specification requires that implementations enable novel syntax within quantum kernels for the ubiquitous **compute-action-uncompute** pattern. Given unitary operations `U`, `V` on `N` qubits, the sequence `U V U^` (here, `U^` is the adjoint of `U`) represents a common pattern across a number of quantum algorithms, where we have **compute**` == U`, **action**` == V`, and **uncompute**` == U^`. Implementations are required to enable the expression of this pattern via keywords in the language extension: **compute** `COMPUTE SCOPE` **action** `ACTION SCOPE`. The intent is to ease expression of redundant quantum code, but also to promote compiler optimizations in the case of `W::ctrl(...), W == U V U^` (here `W::ctrl(...) == U V::ctrl(...) U^` instead of the naive `U::ctrl(...) V::ctrl(...) U^::ctrl(...)`). In a C++ language extension adherent to this specification, this pattern might look like this:
```cpp
int main() {
  // Quantum kernels can be lambdas as well as functions
  auto foo = qpu_lambda([](qreg q) {
     auto N = q.size();
     compute {
        H(q);
        X(q);
     } action {
        Z::ctrl(q.head(), q.tail(N-1))
     }
  };
}
```
which the compiler implementation would expand to `H(q) X(q) Z::ctrl(...) X(q) H(q)`, and any quantum code that called `foo::ctrl(...)` would result only in a control on the existing `controlled-Z`. 

##### <a id="q-unitary"></a>Unitary Matrix Decomposition
The specification requires that implementations enable the expression of circuit synthesis from defined unitary matrices, enabling one to program quantum code at the matrix-level and let the compiler decompose into native gates for the quantum coprocessor. This is enabled via a **decompose** `MATRIX SCOPE` `(ARGS...)`  expression, where `MATRIX SCOPE` contains the code describing the unitary matrix to be decomposed. The explicit circuit synthesis strategy is left up to concrete language implementations. The `ARGS...` must start with the `qubit` or `qreg` that the operation is to be applied to, and can contain any other circuit synthesis pertinent arguments. In a C++ language extension adherent to this specification, this expression might look like this:
```cpp
__qpu__ void foo(qreg q) {
    decompose {
      // Create the unitary matrix
      UnitaryMatrix ccnot_mat = UnitaryMatrix::Identity(8, 8);
      ccnot_mat(6, 6) = 0.0;
      ccnot_mat(7, 7) = 0.0;
      ccnot_mat(6, 7) = 1.0;
      ccnot_mat(7, 6) = 1.0;
    }(q);
}
```  