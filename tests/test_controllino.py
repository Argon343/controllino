import os
import time

import pytest
import serial
import serial.tools.list_ports

from controllino import controllino

WAIT = 0.5
TIMEOUT = 15.0


def get_address_from_serial_number(serial_number: str) -> str:
    """Get device address from USB serial number.

    Arguments:
        serial_number: The device's serial number

    Returns:
        The device's serial port

    """
    devices = serial.tools.list_ports.comports()
    match = next(each for each in devices if each.serial_number == serial_number)
    addr = match.device
    return addr


@pytest.fixture
def api():
    serial_number = os.environ["CONTROLLINO_USB_SERIAL_NUMBER"]
    addr = get_address_from_serial_number(serial_number)
    ser = serial.Serial(port=addr, baudrate=19200)
    time.sleep(0.1)
    ser.reset_input_buffer()  # Cleanup potential spills from previous tests
    _api = controllino.Controllino(ser)
    done = _api.open().wait(WAIT)
    _api.process_errors()
    assert done
    yield _api
    _api.kill()


# FIXME What's the default value of the digital pins?
@pytest.mark.timeout(TIMEOUT)
def test_set_signal_get_signal_digital(api):
    future = api.set_signal("D40", "HIGH")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    future.result()

    future = api.get_signal("D30")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "HIGH"

    future = api.set_signal("D40", "LOW")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    future.result()

    future = api.get_signal("D30")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "LOW"


@pytest.mark.timeout(TIMEOUT)
def test_set_signal_get_signal_analog(api):
    future = api.set_signal("DAC0", 255)
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    future.result()  # Check for errors!

    future = api.get_signal("A0")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() > 800

    future = api.set_signal("DAC0", 0)
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    future.result()  # Check for errors!

    future = api.get_signal("A0")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() < 600


@pytest.mark.timeout(TIMEOUT)
def test_set_pin_mode_get_pin_mode(api):
    future = api.get_pin_mode("D41")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    assert future.result() == "OUTPUT"

    api.set_pin_mode("D41", "INPUT").wait(WAIT)
    api.process_errors()

    future = api.get_pin_mode("D41")
    future.wait(WAIT)
    api.process_errors()
    assert future.result() == "INPUT"


@pytest.mark.timeout(TIMEOUT)
def test_set_pin_mode_set_signal_get_signal(api):
    future = api.set_pin_mode("D43", "INPUT")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.set_signal("D41", "HIGH")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.get_signal("D43")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "HIGH"

    future = api.set_signal("D41", "LOW")
    done = future.wait()
    api.process_errors()
    assert done
    future.result()

    future = api.get_signal("D43")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "LOW"


@pytest.mark.xfail
@pytest.mark.timeout(TIMEOUT)
def test_set_pin_mode_load_save_reset_pin_modes(api):
    future = api.set_pin_mode("D43", "INPUT")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.get_pin_mode("D43")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "INPUT"

    future = api.save_pin_modes()
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.reset_pin_modes()
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.get_pin_mode("D43")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "OUTPUT"

    future = api.load_pin_modes()
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.get_pin_mode("D43")
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    assert future.result() == "INPUT"


class TestSetSignal:
    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin_mode(self, api):
        future = api.set_signal("D30", "HIGH")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_OUTPUT_PIN" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_level(self, api):
        future = api.set_signal("D40", "LO")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_OUTPUT_LEVEL" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin(self, api):
        future = api.set_signal("D99", "HIGH")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN" in str(e.value)


class TestGetSignal:
    @pytest.mark.xfail
    @pytest.mark.timeout(TIMEOUT)
    def test_get_signal_invalid_pin_mode(self, api):
        future = api.get_signal("D40")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_OUTPUT_PIN" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_get_signal_invalid_pin(self, api):
        future = api.get_signal("D99")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN" in str(e.value)


class TestSetPinMode:
    @pytest.mark.xfail
    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin_mode(self, api):
        future = api.set_pin_mode("D41", "INVALID")
        future.wait(WAIT)
        api.process_errors()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN_MODE" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin(self, api):
        future = api.set_pin_mode("D99", "INPUT")
        future.wait(WAIT)
        api.process_errors()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN" in str(e.value)


class TestGetPinMode:
    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin(self, api):
        future = api.get_pin_mode("D99")
        future.wait(WAIT)
        api.process_errors()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN" in str(e.value)


# FIXME `trigger_pulse` is broken, as the device sleeps during the
# pulse, making concurrent logging impossible.
def test_logging_trigger_pulse(api):
    pass


# TODO Test parallel logging jobs


class TestTriggerPulse:
    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin_mode(self, api):
        future = api.trigger_pulse("D30")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_OUTPUT_PIN" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_invalid_pin(self, api):
        future = api.trigger_pulse("D99")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()
        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "INVALID_PIN" in str(e.value)


class CmdUnknown(controllino.Command):
    def _serialize(self):
        return {"command": "UNKNOWN"}


@pytest.mark.xfail
@pytest.mark.timeout(TIMEOUT)
def test_invalid_command(api):
    future = api.submit(CmdUnknown())
    future.wait(WAIT)
    api.process_errors()
    assert future.done()
    with pytest.raises(controllino.ControllinoError) as e:
        future.result()
    assert "INVALID_COMMAND" in str(e.value)


@pytest.mark.timeout(TIMEOUT)
def test_logging_set_signal(api):
    future = api.set_signal("D40", "LOW")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    request, recording = api.log_signal("D30", 500)
    done = request.wait(WAIT)
    api.process_errors()
    assert done
    request.result()

    time.sleep(0.75)
    future1 = api.set_signal("D40", "HIGH")
    if recording.done():  # Raise if recording is unexpectedly done!
        recording.result()

    time.sleep(1.0)
    future2 = api.set_signal("D40", "LOW")
    if recording.done():  # Raise if recording is unexpectedly done!
        recording.result()

    time.sleep(1.0)
    future = api.end_log_signal("D30")
    done = future.wait(WAIT)
    api.process_errors()
    assert done

    done = future1.wait(WAIT)
    api.process_errors()
    assert done
    done = future2.wait(WAIT)
    api.process_errors()
    assert done

    done = recording.wait(5.0)
    api.process_errors()
    assert done
    result = recording.result()
    assert result.values[:6] == [0, 0, 1, 1, 0, 0]


@pytest.mark.timeout(TIMEOUT)
def test_logging_set_signal_concurrent(api):
    # Default levels.
    future = api.set_signal("D40", "LOW")
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    future = api.set_signal("DAC0", 0)
    done = future.wait(WAIT)
    api.process_errors()
    assert done
    future.result()

    request_digital, recording_digital1 = api.log_signal("D30", 500)
    request_analog, recording_analog = api.log_signal("A0", 250)
    recordings = [recording_digital1, recording_analog]

    done = request_digital.wait(WAIT)
    api.process_errors()
    assert done
    request_digital.result()

    done = request_analog.wait(WAIT)
    api.process_errors()
    assert done
    request_analog.result()

    time.sleep(0.60)
    future1_digital = api.set_signal("D40", "HIGH")
    future1_analog = api.set_signal("DAC0", 255)
    for rec in [each for each in recordings if each.done()]:
        rec.result()

    time.sleep(1.0)
    future2_analog = api.set_signal("D40", "LOW")
    for rec in [each for each in recordings if each.done()]:
        rec.result()
    future2_digital = api.end_log_signal("D30")

    time.sleep(1.0)
    request_digital2, recording_digital2 = api.log_signal("D30", 500)
    recordings[0] = recording_digital2
    for rec in [each for each in recordings if each.done()]:
        rec.result()

    done = request_digital2.wait(WAIT)
    api.process_errors()
    assert done
    request_digital2.result()

    time.sleep(1.0)
    future3_analog = api.end_log_signal("A0")
    done = future3_analog.wait(WAIT)
    api.process_errors()
    assert done
    analog_result = future3_analog.result()

    time.sleep(1.0)
    future3_digital = api.end_log_signal("D30")

    done = future1_digital.wait(WAIT)
    api.process_errors()
    assert done
    done = future2_digital.wait(WAIT)
    api.process_errors()
    assert done
    done = future3_digital.wait(WAIT)
    api.process_errors()
    assert done

    done = future1_analog.wait(WAIT)
    api.process_errors()
    assert done
    done = future2_analog.wait(WAIT)
    api.process_errors()
    assert done
    done = future3_analog.wait(WAIT)
    api.process_errors()
    assert done

    done = recording_digital1.wait(WAIT)
    api.process_errors()
    assert done
    result_digital1 = recording_digital1.result()
    assert result_digital1.values[:5] == [0, 0, 1, 1, 0]

    done = recording_digital2.wait(WAIT)
    api.process_errors()
    assert done
    result_digital2 = recording_digital2.result()
    assert set(result_digital2.values) == {0}

    done = recording_analog.wait(WAIT)
    api.process_errors()
    assert done
    result_analog = recording_analog.result()
    assert result_analog.values[0] < 400
    assert result_analog.values[-1] > 600


class TestLogSignal:

    # TODO Add tests for analog pins!
    @pytest.mark.timeout(TIMEOUT)
    @pytest.mark.parametrize(
        "pin, error",
        [
            pytest.param("D40", "INVALID_INPUT_PIN", id="digital output pin"),
            pytest.param("DAC0", "INVALID_INPUT_PIN", id="analog output pin"),
            pytest.param("D99", "INVALID_PIN", id="pin doesn't exist"),
        ],
    )
    def test_invalid_pin_mode(self, api, pin, error):
        request, _ = api.log_signal(pin, 1000)  # OUTPUT pin

        request.wait(WAIT)
        api.process_errors()
        assert request.done()

        with pytest.raises(controllino.ControllinoError) as e:
            request.result()
        assert error in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_duplicate_logging_job(self, api):
        request, _ = api.log_signal("D30", 1000)

        done = request.wait(WAIT)
        api.process_errors()
        assert done
        request.result()

        request, _ = api.log_signal("D30", 500)
        request.wait(WAIT)
        api.process_errors()
        assert request.done()

        with pytest.raises(controllino.ControllinoError) as e:
            request.result()
        assert "DUPLICATE_LOGGING_JOB" in str(e.value)

    @pytest.mark.timeout(TIMEOUT)
    def test_too_many_logging_jobs(self, api):
        pins = {"D" + str(i) for i in range(30, 40)}
        futures = [api.log_signal(each, 1000) for each in pins]

        request, _ = futures[8]
        request.wait(WAIT)
        api.process_errors()
        assert request.done()

        with pytest.raises(controllino.ControllinoError) as e:
            request.result()
        assert "TOO_MANY_LOGGING_JOBS" in str(e.value)


class TestEndLogSignal:
    @pytest.mark.timeout(TIMEOUT)
    def test_no_logging_job_with_that_pin(self, api):
        request, _ = api.log_signal("D30", 500)

        done = request.wait(WAIT)
        api.process_errors()
        assert done
        request.result()

        future = api.end_log_signal("D31")
        future.wait(WAIT)
        api.process_errors()
        assert future.done()

        with pytest.raises(controllino.ControllinoError) as e:
            future.result()
        assert "LOGGING_REQUEST_NOT_FOUND" in str(e.value)
