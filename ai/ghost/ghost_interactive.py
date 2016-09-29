#!//usr/bin/env python3

import sys, cProfile, random
import tty, termios

# word_string = open("large_dict.txt","r").read().lower()
word_string = open("ghost.txt","r").read().lower()
words = word_string.split("\n")

tree = {}

def gentree():
    for word in words:
        curr_tree = tree
        for letter in word:
            if letter not in curr_tree: curr_tree[letter] = {}
            curr_tree = curr_tree[letter]

def is_int(s):
    try:
        int(s)
        return True
    except ValueError:
        return False

def hint(segment):
    curr_tree = tree
    for letter in segment:
        if letter in curr_tree:
            curr_tree = curr_tree[letter]
        else: return []
    return list(curr_tree.keys())

def getch():
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def challenge(segment):
    curr_tree = tree
    for letter in segment:
        if letter not in curr_tree:
            return False
        curr_tree = curr_tree[letter]
    return True

def challenge_input(word):
    return word in words

def pick_letter(word):
    ct = tree
    for l in word:
        if l not in ct: return '!'
        ct = ct[l]
    possible = hint(word)
    if len(possible) > 0:
        scrambled = possible[:]
        random.shuffle(scrambled)
        for l in scrambled:
            if len(ct[l]) > 0:
                return l
        return possible[0]
    if len(word) < 4:
        return "s" # unlikely
    return '!'

def challenge_player(word, p, lp, players):
    print("")
    print("Player {} challenged {}".format(p+1, lp+1))
    if players[lp] == 'c':
        if word not in words:
            ct = tree[word[0]]
            for l in word[1:]:
                if l in ct: ct = ct[l]
                else: ct = {}
            word_to_challenge = word
            while len(ct) > 0:
                ltp = list(ct.keys())[0]
                word_to_challenge += ltp
                ct = ct[ltp]
            print("Computer player {} used {} to refute".format(lp+1, word_to_challenge))
            if word_to_challenge in words:
                print("Player {} loses!".format(p+1))
                return False
            else:
                print("Player {} wins!".format(p+1))
                return True
        else:
            print("Player {} wins!".format(p+1))
            return True

    else:
        if word in words or not challenge_input(word+input("Word to refute challenge: "+word)) or not challenge(word):
            print("Player {} wins!".format(p+1))
            return True
        else:
            print("Player {} loses!".format(p+1))
            return False

def ghost(player_list):
    num_players = len(player_list)
    sys.stdout.write("Loading...\r")
    sys.stdout.flush()
    gentree()
    alphabet = "abcdefghijklmnopqrstuvwxyz"
    players = [chr(ord('1') + i) for i in range(num_players)]
    print("Press ESC to exit")
    word = ""
    can_backspace = True
    backspace = {8,127}
    quit = {3,27,28}
    player = 0
    last_player = -1
    players_removed = 0
    while True:
        player = player%num_players
        last_player = last_player%num_players
        if players_removed == num_players - 1:
            print("\nRan out of players")
            return
        if player_list[player] == 'r':
            player += 1
        elif player_list[player] == 'c':
            letter = "!"
            if len(word) < 4 or word not in words:
                letter = pick_letter(word)
            if letter == '!':
                if challenge_player(word, player, last_player, player_list):
                    player_list[last_player] = 'r'
                else:
                    player_list[player] = 'r'
                return # end game
                players_removed += 1
                sys.stdout.write(word)
                sys.stdout.flush()
            else:
                sys.stdout.write(letter)
                sys.stdout.flush()
            word += letter
            last_player = player
            player += 1
        else:
            key = getch().lower()
            num = ord(key)
            if key == '.':
                print("\rPossible choices: " + ", ".join(sorted(hint(word))))
                sys.stdout.write(word)
                sys.stdout.flush()
            """
            if num in backspace:
                if can_backspace:
                    player -= 1
                    word = word[:-1]
                    sys.stdout.write(key + " " + key)
                    sys.stdout.flush()
                    can_backspace = False
                else:
                    sys.stdout.write(chr(7))
                    sys.stdout.flush()
            """
            if num in quit:
                print()
                return
            if key in players and int(key)%num_players != player and len(word) > 3:
                if challenge_player(word, int(key) - 1, last_player, player_list):
                    player_list[int(key) - 1] = 'r'
                else:
                    player_list[last_player] = 'r'
                return 
                players_removed += 1
                sys.stdout.write(word)
                sys.stdout.flush()
            if key in alphabet:
                can_backspace = True
                word += key
                sys.stdout.write(key)
                sys.stdout.flush()
                last_player = player
                player += 1
    return

def main():
    if(len(sys.argv) == 1):
        print("Specify players")
        return
    players = []
    for arg in sys.argv[1:]:
        if arg.lower() == 'c':
            players.append("c")
        elif is_int(arg) and int(arg) in range(1,10):
            for i in range(int(arg)):
                players.append("p")
        else:
            print("Invalid argument {}".format(arg))
            return
    ghost(players)

main()
