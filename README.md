# AccDec_8Coil

DCC accessory decoder to drive 8 electromagnetic coils.

Originally derived from an example of library NmraDcc.

Library required:

- [NmraDcc] (public)
- [ConfCVlib] (manual installation)
- [DccSerialCom] (manual installation)

Features:

- 8 available outputs for "double coils" switches (common positive).
- output drive stages with ULN2803 chips.
- customizable pulse time or continuos mode in case to drive light bulbs.
- fail safe time to avoid coils overcharging and burnout.
- output voltage up to 20 Vdc.
- Separate power socket or autopower via DCC.
- Sigle address mode (continuos address for each output) or multiple address mode (each output has it own address).

Designed to be configured with the custom PC tool [DecoderConfigurator], or in standardize way via CV.

[NmraDcc]: https://github.com/mrrwa/NmraDcc
[ConfCVlib]: https://github.com/M5Ross/ConfCVlib
[DccSerialCom]: https://github.com/M5Ross/DccSerialCom
[DecoderConfigurator]: https://github.com/M5Ross/DecoderConfigurator