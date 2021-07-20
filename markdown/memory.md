---
title: "Memory Model"
date: 2019-11-29T15:26:15Z
draft: false
weight: 15
---


The qcor memory model assumes two classical memory spaces: (1) the host memory space, and (2) the memory space leveraged by the classical control electronics driving the execution of quantum operations. The model assumes an infinite-sized global register of quantum bits (qubits), and that programmers can allocate sub-registers of this global register via allocation library calls. The qcor memory model therefore consists of both classical and quantum components. The classical host memory space should adhere to the memory model of the language being extended (C++, Python, etc.). 

The control-system memory space should be opaque to the programmer. Its role is to store qubit measurement results for retrieval by the host CPU (extract quantum program execution results), or for conditional execution of subsequent quantum expressions. 

The memory model specifies an opaque `qubit` type that programmers can allocate and reference. This type models a single quantum bit in the assumed global register provided by the quantum memory space. Intrinsic quantum instructions operate on instances of the `qubit` type. Collections of `qubits` or sub-registers are to be expressed as another defined type: the `qreg`. The `qreg` models a continuous register of quantum memory and exposes an API for further sub-register extraction and individual `qubit` addressing (see [Programming Model](prog_model.md)). Moreover, the `qreg` provides programmers with a handle on quantum program execution results after invocation of intrinsic quantum and measurement operations have been invoked.

Programmers primarily allocate `qreg` instances within host CPU code. `qubit` instance can be addressed individually from the allocated `qreg`. All allocated `qubit` instances start in the `|0>` computational basis state. `qreg` (and addressed `qubit`) instances are meant to be allocated in host language code and passed (implicitly by reference) as input to defined quantum kernel functions. If the kernel is an entry-point kernel (invoked from host code), execution results will be persisted to the allocated `qreg` instance that was operated upon.

Programmers can allocate temporary `qreg` instances on the stack within quantum kernel definitions. This is often useful for the allocation and utility of ancilla `qubits` within certain quantum algorithms. All kernel stack-allocated `qubit` instances start in the `|0>` state. When the `qubit` or `qreg` goes out of scope, the default behavior is assumed to be that implicit uncomputation of the stack-allocated `qubit` will be invoked, returning stack-allocated `qubit` instances to the `|0>` state. The programmer can indicate at allocation that uncomputation should not be performed, and in this case, it is on the programmer to ensure that stack-allocated `qubit` deallocation is properly handled. 