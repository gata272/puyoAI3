# Ama evaluation compatibility

The current implementation uses the public `citrus610/ama` `build` weight profile.

The public ama evaluation has 15 weights:

`chain, y, key, chi, shape, well, bump, form, link_2, link_3, waste_14, side, nuisance, tear, waste`.

The exact public static formulas for `shape`, `well`, `bump`, `side` and `waste_14` are reproduced conceptually in this project. The public ama implementation also uses a bit-field definition for `link_2/link_3`; this project uses a scalar board representation with an equivalent feature intent rather than copying the SIMD bit-field implementation.

`form` is intentionally a small GTR-oriented pattern detector in this repository. Ama's complete human-form library is substantially larger and is represented by its private pattern data files; this repository does not claim bit-for-bit form compatibility.

The `chain/y/key/chi` terms are evaluated by the project's quiescence search. Their purpose follows ama's public implementation, but the internal `quiet::search` and `key` calculation are not copied verbatim.

Therefore the project should currently be described as:

> GTR opening planner + ama-weighted, ama-inspired linear evaluation + beam/quiescence search

rather than as a bit-for-bit reimplementation of ama.

The exact public weights are stored in `config/weights.json`.
