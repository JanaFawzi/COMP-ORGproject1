# ZX16 Snake
# Controls: Up/Down/Left/Right or W/A/S/D
# Tile map: 0xF000, 20 x 15
# Tile IDs: 0 empty, 1 snake, 2 food, 3 wall
#
# Interview overview:
# - The tile map is both the display and the collision grid: map[offset]
#   tells us whether the next cell is empty, snake, food, or wall.
# - Each snake segment is stored as a 16-bit tile-map offset in `segments`.
#   The tail is at index 0 and the head is at index length - 1.
# - Direction is an offset added to the head: up=-20, down=20,
#   left=-1, right=1. Walls prevent horizontal row wrapping.
# - Important ecalls: 0x030 reads input, 0x032 returns randomness,
#   0x020 plays a tone, and 0x012 prints a string.

.text

start:
    # Keep the stack below memory-mapped video/palette memory.
    li16 sp, 0xEFFE

    # Configure the display for the 20 x 15 tile-map mode.
    li a0, 35
    ecall 0x021

    # Palette memory begins at 0xFA00. Each byte defines one color used
    # by the packed tile pixels below: black, green, red, and white.
    li16 x3, 0xFA00
    li x4, 0x00
    sb x4, 0(x3)
    li x4, 0x1C
    sb x4, 1(x3)
    li x4, 0xE0
    sb x4, 2(x3)
    li x4, 0xFF
    sb x4, 3(x3)

    # Tile graphics begin at 0xF200. A tile occupies 128 bytes, so each
    # loop writes exactly one complete 8 x 8 tile definition.
    # Tile 0: empty (all pixels use palette color 0).
    li16 x3, 0xF200
    li16 x4, 128
    li x5, 0
fill_tile0:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile0

    # Tile 1: snake, full green block. 0x11 packs two color-1 pixels
    # into each byte.
    li16 x4, 128
    li x5, 0x11
fill_tile1:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile1

    # Tile 2: first clear the tile, then draw an eight-row red center.
    li16 x4, 128
    li x5, 0
fill_tile2:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile2

    li16 x3, 0xF304
    li x4, 8
    li x5, 0x22
food_tile_rows:
    # Four adjacent bytes form the apple's center on each row; 0x22
    # selects palette color 2 for both packed pixels.
    sb x5, -2(x3)
    sb x5, -1(x3)
    sb x5, 0(x3)
    sb x5, 1(x3)
    addi x3, 8
    dec x4
    bnz x4, food_tile_rows

    # Tile 3: solid white wall (two color-3 pixels per byte).
    li16 x3, 0xF380
    li16 x4, 128
    li x5, 0x33
fill_tile3:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile3

    # Clear all 20 * 15 = 300 visible map cells before placing objects.
    li16 x3, 0xF000
    li16 x4, 300
    li x5, 0
clear_map:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, clear_map

    # Draw top and bottom borders in parallel. 0xF118 is map offset 280,
    # the first cell of the final 20-cell row.
    li16 x3, 0xF000
    li16 x6, 0xF118
    li x4, 20
    li x5, 3
wall_top_bottom:
    sb x5, 0(x3)
    sb x5, 0(x6)
    addi x3, 1
    addi x6, 1
    dec x4
    bnz x4, wall_top_bottom

    # Draw the first and last cell of every row. Advancing by 20 moves
    # the pointer down one row.
    li16 x3, 0xF000
    li x4, 15
wall_sides:
    sb x5, 0(x3)
    mv x6, x3
    addi x6, 19
    sb x5, 0(x6)
    addi x3, 20
    dec x4
    bnz x4, wall_sides

    # Initial body: four consecutive cells on row 7. Increasing offsets
    # run left-to-right, so offset 151 is the starting head.
    li16 x3, segments
    li16 x4, 148
    sw x4, 0(x3)
    li16 x4, 149
    sw x4, 2(x3)
    li16 x4, 150
    sw x4, 4(x3)
    li16 x4, 151
    sw x4, 6(x3)

    li16 x3, length
    # Game variables live in RAM; x3 commonly holds their address and
    # x4/x5/x6 are temporary values throughout this program.
    li x4, 4
    sw x4, 0(x3)

    li16 x3, direction
    li x4, 1
    sw x4, 0(x3)

    li16 x3, next_food_offset
    li x4, 72
    sw x4, 0(x3)

    li16 x3, score
    li x4, 0
    sw x4, 0(x3)

    # Draw the same initial offsets into video memory as snake tile 1.
    li x4, 1
    li16 x3, 0xF094
    sb x4, 0(x3)
    sb x4, 1(x3)
    sb x4, 2(x3)
    sb x4, 3(x3)

init_food0:
    # Start at a random byte-sized offset, then scan forward until an
    # empty tile is found. The scan wraps after the 300-cell map.
    ecall 0x032
    li16 x3, rng_byte
    sb a0, 0(x3)
    lbu x4, 0(x3)
    j init_food0_try
init_food0_try:
    li16 x3, 0xF000
    add x3, x4
    lbu x5, 0(x3)
    li x6, 0
    beq x5, x6, init_food0_place
    addi x4, 1
    li16 x5, 300
    bltu x4, x5, init_food0_loop
    li x4, 0
init_food0_loop:
    j init_food0_try
init_food0_place:
    li16 x3, food0
    sw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 2
    sb x5, 0(x3)

init_food1:
    # Repeat the placement algorithm for the second apple. Since the map
    # is checked first, it cannot overlap the snake, walls, or apple 0.
    ecall 0x032
    li16 x3, rng_byte
    sb a0, 0(x3)
    lbu x4, 0(x3)
    j init_food1_try
init_food1_try:
    li16 x3, 0xF000
    add x3, x4
    lbu x5, 0(x3)
    li x6, 0
    beq x5, x6, init_food1_place
    addi x4, 1
    li16 x5, 300
    bltu x4, x5, init_food1_loop
    li x4, 0
init_food1_loop:
    j init_food1_try
init_food1_place:
    li16 x3, food1
    sw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 2
    sb x5, 0(x3)

init_food2:
    # Place a third independent apple using the same collision-safe scan.
    ecall 0x032
    li16 x3, rng_byte
    sb a0, 0(x3)
    lbu x4, 0(x3)
    j init_food2_try
init_food2_try:
    li16 x3, 0xF000
    add x3, x4
    lbu x5, 0(x3)
    li x6, 0
    beq x5, x6, init_food2_place
    addi x4, 1
    li16 x5, 300
    bltu x4, x5, init_food2_loop
    li x4, 0
init_food2_loop:
    j init_food2_try
init_food2_place:
    li16 x3, food2
    sw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 2
    sb x5, 0(x3)

    # After this, wait for the first key press.
wait_for_start:
    # Input codes 1..4 represent the four movement directions.
    ecall 0x030
    li x5, 1
    beq a0, x5, start_game
    li x5, 2
    beq a0, x5, start_game
    li x5, 3
    beq a0, x5, start_game
    li x5, 4
    beq a0, x5, start_game
    j wait_for_start

start_game:
    j input_ready

game_loop:
    # A busy-wait controls game speed; one move is processed per tick.
    li16 x3, 3000
delay_loop:
    dec x3
    bnz x3, delay_loop

    ecall 0x030

input_ready:
    # Reject a 180-degree turn: reversing directly would make the new
    # head collide with the segment immediately behind it.
    li x5, 1
    beq a0, x5, set_up
    j check_down
set_up:
    # Current direction 20 means down, the forbidden opposite of up.
    li16 x3, direction
    lw x6, 0(x3)
    li x5, 20
    bne x6, x5, set_up_apply
    j direction_done
set_up_apply:
    li x4, -20
    sw x4, 0(x3)
    j direction_done

check_down:
    li x5, 2
    beq a0, x5, set_down
    j check_left
set_down:
    # Current direction -20 means up, the forbidden opposite of down.
    li16 x3, direction
    lw x6, 0(x3)
    li x5, -20
    bne x6, x5, set_down_apply
    j direction_done
set_down_apply:
    li x4, 20
    sw x4, 0(x3)
    j direction_done

check_left:
    li x5, 3
    beq a0, x5, set_left
    j check_right
set_left:
    # Current direction 1 means right, the forbidden opposite of left.
    li16 x3, direction
    lw x6, 0(x3)
    li x5, 1
    bne x6, x5, set_left_apply
    j direction_done
set_left_apply:
    li x4, -1
    sw x4, 0(x3)
    j direction_done

check_right:
    li x5, 4
    beq a0, x5, set_right
    j direction_done
set_right:
    # Current direction -1 means left, the forbidden opposite of right.
    li16 x3, direction
    lw x6, 0(x3)
    li x5, -1
    bne x6, x5, set_right_apply
    j direction_done
set_right_apply:
    li x4, 1
    sw x4, 0(x3)

direction_done:
    # Compute candidate head = current head + direction.
    li16 x3, length
    lw x5, 0(x3)
    dec x5
    slli x5, 1
    li16 x3, segments
    add x3, x5
    lw x4, 0(x3)

    li16 x3, direction
    lw x5, 0(x3)
    add x4, x5

    li16 x3, pending_head
    sw x4, 0(x3)

    li16 x3, 0xF000
    add x3, x4
    # Reading the destination tile makes collision detection O(1).
    lbu x5, 0(x3)

    li x6, 0
    bne x5, x6, check_food
    j normal_move

check_food:
    # Destination tile 2 grows the snake; any value not handled as empty,
    # food, or a legal tail move is a fatal collision (normally a wall).
    li x6, 2
    bne x5, x6, check_snake
    j eat_food

check_snake:
    li x6, 1
    beq x5, x6, snake_tile
    j game_over
snake_tile:
    # Moving onto the current tail is legal: during a normal move that
    # tile is erased before the new head is drawn. Any other snake tile
    # is a self-collision.
    li16 x3, segments
    lw x6, 0(x3)
    beq x4, x6, tail_tile
    j game_over
tail_tile:
    j normal_move

normal_move:
    # A non-growing move is: erase tail, shift offsets left, append head.
    # Erase old tail.
    li16 x3, segments
    lw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 0
    sb x5, 0(x3)

    # Shift body one slot toward the tail.
    li16 x3, length
    lw x5, 0(x3)
    dec x5
    li16 x3, segments
shift_loop:
    # Copy segments[i + 1] into segments[i]. Each entry is two bytes,
    # which explains the +2 pointer step.
    mv x6, x3
    addi x6, 2
    lw a1, 0(x6)
    sw a1, 0(x3)
    addi x3, 2
    dec x5
    bnz x5, shift_loop

    # Store new head in the last slot.
    li16 x3, length
    lw x5, 0(x3)
    dec x5
    slli x5, 1
    li16 x3, segments
    add x3, x5
    li16 x4, pending_head
    lw x4, 0(x4)
    sw x4, 0(x3)
    j draw_head

eat_food:
    # Append a new head and increase length, up to 80 cells.
    li16 x3, length
    lw x5, 0(x3)
    li16 x6, 80
    bltu x5, x6, grow_body
    j max_length_food_move

grow_body:
    # Growth needs no body shift: preserve the old tail and append the
    # pending head in the first unused segment slot.
    mv x6, x5
    slli x6, 1
    li16 x3, segments
    add x3, x6
    li16 x4, pending_head
    lw x4, 0(x4)
    sw x4, 0(x3)

    inc x5
    li16 x3, length
    sw x5, 0(x3)

    li16 a0, 880
    li16 a1, 90
    # Eating feedback: play an 880 Hz tone for 90 ms.
    ecall 0x020

    li16 x3, score
    # Each apple is worth five points.
    lw x5, 0(x3)
    addi x5, 5
    sw x5, 0(x3)

    j draw_head_after_food

max_length_food_move:
    # At max length, still move forward and replace the eaten apple.
    # This path mirrors normal_move because no additional segment slot
    # is available, but eating and scoring still occur.
    li16 x3, segments
    lw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 0
    sb x5, 0(x3)

    li16 x3, length
    lw x5, 0(x3)
    dec x5
    li16 x3, segments
max_food_shift_loop:
    mv x6, x3
    addi x6, 2
    lw a1, 0(x6)
    sw a1, 0(x3)
    addi x3, 2
    dec x5
    bnz x5, max_food_shift_loop

    li16 x3, length
    lw x5, 0(x3)
    dec x5
    slli x5, 1
    li16 x3, segments
    add x3, x5
    li16 x4, pending_head
    lw x4, 0(x4)
    sw x4, 0(x3)

    li16 a0, 880
    li16 a1, 90
    ecall 0x020

    li16 x3, score
    lw x5, 0(x3)
    addi x5, 5
    sw x5, 0(x3)

    j draw_head_after_food

draw_head:
    # Convert the stored map offset back into a video-memory address and
    # write tile ID 1. The display updates directly from mapped memory.
    li16 x3, pending_head
    lw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 1
    sb x5, 0(x3)
    j game_loop

draw_head_after_food:
    li16 x3, pending_head
    lw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 1
    sb x5, 0(x3)
    j food_next

food_next:
    # Replace only the apple that was eaten; the other two stay put.
    # As during initialization, linear probing guarantees an empty cell
    # is selected even when the random starting cell is occupied.
    ecall 0x032
    li16 x3, rng_byte
    sb a0, 0(x3)
    lbu x4, 0(x3)
    j food_try

food_wrap:
    # Linear probing reached the end of the map; continue from cell 0.
    li x4, 0

food_try:
    li16 x3, 0xF000
    add x3, x4
    lbu x5, 0(x3)
    li x6, 0
    beq x5, x6, food_place
    j food_advance

food_advance:
    addi x4, 1
    li16 x5, 300
    bltu x4, x5, food_advance_try
    j food_wrap

food_advance_try:
    j food_try

food_place:
    # pending_head is the eaten apple's old position. Compare it with the
    # three saved apple positions to identify which record must change.
    li16 x3, pending_head
    lw x5, 0(x3)

    li16 x3, food0
    lw x6, 0(x3)
    beq x5, x6, replace_food0
    j check_replace_food1

replace_food0:
    sw x4, 0(x3)
    j save_next_food

check_replace_food1:
    li16 x3, food1
    lw x6, 0(x3)
    beq x5, x6, replace_food1
    j replace_food2

replace_food1:
    sw x4, 0(x3)
    j save_next_food

replace_food2:
    li16 x3, food2
    sw x4, 0(x3)

save_next_food:
    # Save the replacement position, render tile 2 there, and return to
    # the loop. `jr` is used because this ISA loads the label address.
    li16 x3, next_food_offset
    sw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 2
    sb x5, 0(x3)
    li16 x3, game_loop
    jr x3

game_over:
    # Play the failure tone, print the final score, then halt the VM.
    li16 a0, 180
    li16 a1, 700
    ecall 0x020

    li16 a0, game_over_text
    ecall 0x012
    li16 x3, score
    lw a0, 0(x3)
    ecall 0x000
    li a0, 10
    ecall 0x001
    ecall 0x3FF

.data
# Persistent game state. Words are 16-bit on ZX16, hence the two-byte
# stride used when indexing `segments` and its 160-byte capacity (80 cells).
length:           .word 4
direction:        .word 1       # Signed map delta for the next move.
pending_head:     .word 151     # Candidate head, before collision handling.
next_food_offset: .word 72      # Most recently selected food position.
score:            .word 0       # Five points per apple.
food0:            .word 0       # Current position of apple 0.
food1:            .word 0       # Current position of apple 1.
food2:            .word 0       # Current position of apple 2.
rng_byte:         .word 0       # Low random byte used as a map start index.
segments:         .space 160    # 80 segment offsets * 2 bytes each.
game_over_text:   .string "GAME OVER  SCORE: "
