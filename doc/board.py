def changestate(state, white, black):
    if (state == white):
        return black
    else:
        return white

border = '#'
black = '*'
white = ' '
width = 8 
height = 4 
width_by_2 = width // 2
height_by_2 = height // 2
state = black
for i in range(height * 8 + 1):
    if (i % height == height_by_2):
        print(' ' * (width_by_2 - 1), end = '')
        print(8 - (i // height), end = '')
        print(' ' * (width_by_2 - 1), end = '')
    else:
        print(' ' * (width - 1), end = '')
    if (i % height == 0):
        for j in range(8 * width + 1):
            print(border, end = '')
        state = changestate(state, white, black)
    else:
        state = changestate(state, white, black)
        for j in range(8 * width + 1):
            if (j % width == 0):
                state = changestate(state, white, black)
                print(border, end = '')
            elif (i % height == height_by_2 and j % width == width_by_2):
                print('%', end = '')
            else:
                print(state, end = '')
    print()


