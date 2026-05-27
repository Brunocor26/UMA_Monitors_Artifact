# pulse_reader

NuttX built-in application that counts rising edges on a GPIO pin and reports frequency (Hz) and rotational speed (RPM).

## How it works

The app polls the target GPIO driver at ~10 kHz (100 µs sleep), detects rising-edge transitions, and at the end of each time window prints:

```
RPM= <value>
Frequency= <value>Hz
```

RPM is computed as:

```
rpm = (pulses_in_window * 60) / (time_window_seconds * ppr)
```

where `ppr` (pulses per revolution) is hard-coded to **2**.

## Usage

```
pulse_app [-t <seconds>] <gpio-driver-path>
```

| Argument | Default | Description |
|---|---|---|
| `-t <seconds>` | `1` | Measurement window length |
| `<gpio-driver-path>` | — | Full path to the NuttX GPIO driver (e.g. `/dev/gpio0`) |

## Integration into NuttX

Place `PulseApp.c` in your NuttX `CustomApps` directory and register it as a built-in application following the standard NuttX application build process (`Makefile` + `Kconfig` entries).

The GPIO driver must support the `GPIOC_READ` ioctl from `<nuttx/ioexpander/gpio.h>`.
