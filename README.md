# Controllino

Copyright (c) 2022 8tronix

Copyright (c) 2022 Forschungs- und Entwicklungszentrum FH Kiel GmbH

## Building and flashing

Building the software has no requirements.

In order to flash the software, you need to create the `platformio.ini`. In
order to do this, set the environment variable
`CONTROLLINO_USB_SERIAL_NUMBER_DEVICE` to the usb serial number of the target
device (see [Finding USB serial numbers](#Fining-usb-serial-numbers) for
details), then run `make freeze` to freeze the port of the device into the
`platformio.ini`. Repeat this whenever the device changes ports.

When this is done, run `make flash` (`make flash` will also run `make freeze`).

## Testing

For testing, we use `pytest` and the controllino Python API. You need to install
`pytest`, `pyserial` and [python-controllino] via pip:

```shell
pip install -r requirements.txt
```

Testing requires one Arduino Due board. Please make the following connections
before running the tests:

-   `DAC0` to `A0`
-   `D41` to `D43`
-   `D30` to `D40`

To run the tests, execute `make flash`, then `make test`.

## Finding USB serial numbers

You can discover the serial number by running the following python code
(requires `pip install tabulate`, in addition to `pyserial`):

```python
from serial.tools.list_ports import comports
from tabulate import tabulate
devices = comports()
data = [(each.description, each.device, each.serial_number)
        for each in devices]
tab = tabulate(data, headers=['Description', 'Device', 'Serial number'])
print(tab)
```

## Specification

Baudrate must be `19200`.


<!-- Links -->

[python-controllino]: https://github.com/maltekliemann/python-controllino
