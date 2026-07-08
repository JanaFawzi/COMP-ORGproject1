# ZX16 Snake
# Controls: Up/Down/Left/Right or W/A/S/D
# Tile map: 0xF000, 20 x 15
# Tile IDs: 0 empty, 1 snake, 2 food, 3 wall

.text

start:
    # Palette
    li16 x3, 0xFA00
    li x4, 0x00
    sb x4, 0(x3)
    li x4, 0x1C
    sb x4, 1(x3)
    li x4, 0xE0
    sb x4, 2(x3)
    li x4, 0xFF
    sb x4, 3(x3)

    # Tile 0: empty
    li16 x3, 0xF200
    li16 x4, 128
    li x5, 0
fill_tile0:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile0

    # Tile 1: snake, full green block
    li16 x4, 128
    li x5, 0x11
fill_tile1:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile1

    # Tile 2: apple, smaller red center
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
    sb x5, -2(x3)
    sb x5, -1(x3)
    sb x5, 0(x3)
    sb x5, 1(x3)
    addi x3, 8
    dec x4
    bnz x4, food_tile_rows

    # Tile 3: wall
    li16 x3, 0xF380
    li16 x4, 128
    li x5, 0x33
fill_tile3:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, fill_tile3

    # Clear map
    li16 x3, 0xF000
    li16 x4, 300
    li x5, 0
clear_map:
    sb x5, 0(x3)
    addi x3, 1
    dec x4
    bnz x4, clear_map

    # Horizontal walls
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

    # Vertical walls
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

    # Initial body: offsets 148,149,150,151
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

    li16 x3, food0
    li16 x4, 170
    sw x4, 0(x3)

    li16 x3, food1
    li16 x4, 207
    sw x4, 0(x3)

    li16 x3, food2
    li16 x4, 228
    sw x4, 0(x3)

    # Initial draw. After this, wait for the first key press.
    li x4, 2
    li16 x3, 0xF0AA
    sb x4, 0(x3)
    li16 x3, 0xF0CF
    sb x4, 0(x3)
    li16 x3, 0xF0E4
    sb x4, 0(x3)

    li x4, 1
    li16 x3, 0xF094
    sb x4, 0(x3)
    sb x4, 1(x3)
    sb x4, 2(x3)
    sb x4, 3(x3)

wait_for_start:
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
    li16 x3, 3000
delay_loop:
    dec x3
    bnz x3, delay_loop

    ecall 0x030

input_ready:
    li x5, 1
    beq a0, x5, set_up
    j check_down
set_up:
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
    li16 x3, direction
    lw x6, 0(x3)
    li x5, -1
    bne x6, x5, set_right_apply
    j direction_done
set_right_apply:
    li x4, 1
    sw x4, 0(x3)

direction_done:
    # x4 = new head offset
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
    lbu x5, 0(x3)

    li x6, 0
    bne x5, x6, check_food
    j normal_move

check_food:
    li x6, 2
    bne x5, x6, check_snake
    j eat_food

check_snake:
    li x6, 1
    beq x5, x6, snake_tile
    j game_over
snake_tile:
    li16 x3, segments
    lw x6, 0(x3)
    beq x4, x6, tail_tile
    j game_over
tail_tile:
    j normal_move

normal_move:
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
    # Append a new head and increase length, up to 24 cells.
    li16 x3, length
    lw x5, 0(x3)
    li x6, 24
    bltu x5, x6, grow_body
    j normal_move

grow_body:
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

    li16 x3, score
    lw x5, 0(x3)
    inc x5
    sw x5, 0(x3)

    j draw_head_after_food

draw_head:
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
    li16 x3, next_food_offset
    lw x4, 0(x3)
    addi x4, 37

    li16 x5, 260
    bltu x4, x5, food_try
    j food_wrap

food_wrap:
    li x4, 42

food_try:
    li16 x3, 0xF000
    add x3, x4
    lbu x5, 0(x3)
    li x6, 0
    beq x5, x6, food_place
    j food_advance

food_advance:
    addi x4, 37
    li16 x5, 260
    bltu x4, x5, food_try
    j food_wrap

food_place:
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
    li16 x3, next_food_offset
    sw x4, 0(x3)
    li16 x3, 0xF000
    add x3, x4
    li x5, 2
    sb x5, 0(x3)
    j game_loop

game_over:
    li16 a0, game_over_text
    ecall 0x012
    li16 x3, score
    lw a0, 0(x3)
    ecall 0x000
    li a0, 10
    ecall 0x001
    ecall 0x3FF

.data
length:           .word 4
direction:        .word 1
pending_head:     .word 151
next_food_offset: .word 72
score:            .word 0
food0:            .word 170
food1:            .word 207
food2:            .word 228
segments:         .space 64
game_over_text:   .string "GAME OVER  SCORE: "
