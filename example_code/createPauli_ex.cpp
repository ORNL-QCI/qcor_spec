auto H = qcor::createPauli("pauli-operator", 
      "5.907 - 2.1433 X0X1 - 2.1433 Y0Y1 + .21829 Z0 - 6.125 Z1");

auto H = 5.907 - 2.1433 * qcor::X(0) * qcor::X(1) -
           2.1433 * qcor::Y(0) * qcor::Y(1) + .21829 * qcor::Z(0) -
           6.125 * qcor::Z(1);

