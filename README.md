# blockchain-iot-lib

Work done for the paper "A Blockchain-Based Framework for Secure IoT Device Communication"

## Authors

- [**Martin D'hérouville**](https://github.com/mazettt) <img src="https://github.com/mazettt.png" alt="pp-mazettt" width="20" height="20">
- [**Lucas Hauszler**](https://github.com/ripel2) <img src="https://github.com/ripel2.png" alt="pp-ripel2" width="20" height="20">
- [**Thomas Mazaud**](https://github.com/Fyroeo) <img src="https://github.com/Fyroeo.png" alt="pp-fyoreo" width="20" height="20">

## Prerequisites

The following tools should be installed:
- make
- g++

The following libraries need to be installed systemwide:
- openssl
- secp256k1
- paho-mqttpp3
- paho-mqtt3a
- gtest

## How to build

Build the library:
```bash
make
```

Build and run the simulation:
```bash
make simulation
./run_simulation
```

Build and run the metrics:
```bash
make metrics
./run_metrics
```

Build and run the unit tests:
```bash
make unit_tests
./run_unit_tests
```

If the shared library is not found, use the following command:
```bash
export LD_LIBRARY_PATH=./:${LD_LIBRARY_PATH}
```
Make sure that this is used only with this library, as this will add `./` to the shared libraries paths for the current terminal.
