.PHONY: default
default: freeze
	platformio run -e arduinodue

.PHONY: test
test:
	pip install -r requirements.txt
	pytest -vv tests/

.PHONY: freeze
freeze:
	./freeze_device

.PHONY: clean
clean:
	platformio run --target clean

.PHONY: flash
flash: freeze
	platformio run -e arduinodue -t upload
