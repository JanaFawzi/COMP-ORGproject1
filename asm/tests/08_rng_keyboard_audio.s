# Test 08: RNG, keyboard, and audio services
# It checks deterministic RNG output, keyboard no-key reads, and calls audio ECALLs.

.text

start:
    li16 a0, title
    ecall 0x012

    # The default seed 0xACE1 must produce the documented xorshift sequence.
    li16 a0, 0xACE1
    ecall 0x031

    ecall 0x032
    li16 x3, 0xD30F
    beq a0, x3, rng_1_ok
    j rng_fail
rng_1_ok:
    ecall 0x032
    li16 x3, 0xF1A5
    beq a0, x3, rng_2_ok
    j rng_fail
rng_2_ok:
    ecall 0x032
    li16 x3, 0x1734
    beq a0, x3, rng_3_ok
    j rng_fail
rng_3_ok:
    li16 a0, rng_pass
    ecall 0x012

    # A zero seed is forced back to the documented default seed.
    li a0, 0
    ecall 0x031
    ecall 0x032
    li16 x3, 0xD30F
    beq a0, x3, zero_seed_ok
    j rng_fail
zero_seed_ok:
    li16 a0, zero_seed_pass
    ecall 0x012

    # In an unattended assembly run no key is pressed, so keyboard returns 0.
    ecall 0x030
    li x3, 0
    beq a0, x3, keyboard_ok
    j keyboard_fail
keyboard_ok:
    li16 a0, keyboard_pass
    ecall 0x012

    # Audio services are stateful in the host. This program issues valid
    # requests; the C++ unit tests inspect the pending tone/volume/stop state.
    li16 a0, 440
    li16 a1, 250
    ecall 0x020
    li a0, 50
    ecall 0x021
    ecall 0x022
    li16 a0, audio_pass
    ecall 0x012

    li16 a0, all_pass
    ecall 0x012
    ecall 0x3FF

rng_fail:
    li16 a0, rng_bad
    ecall 0x012
    ecall 0x3FF
keyboard_fail:
    li16 a0, keyboard_bad
    ecall 0x012
    ecall 0x3FF

.data
title:          .string "TEST 08 - RNG KEYBOARD AUDIO\n"
rng_pass:       .string "RNG DEFAULT SEQUENCE: PASS\n"
zero_seed_pass: .string "RNG ZERO SEED: PASS\n"
keyboard_pass:  .string "KEYBOARD NO KEY: PASS\n"
audio_pass:     .string "TONE VOLUME STOP AUDIO: PASS\n"
all_pass:       .string "TEST 08 RESULT: PASS\n"
rng_bad:        .string "TEST 08 RESULT: FAIL AT RNG\n"
keyboard_bad:   .string "TEST 08 RESULT: FAIL AT KEYBOARD\n"
