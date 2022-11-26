from enum import Enum
import anytree as tree

class Player(Enum):
    FIRST = 0
    SECOND = 1

    def get_opponent(self):
        if self == Player.FIRST:
            return Player.SECOND
        else:
            return Player.FIRST

class GameState:
    turn : int
    boxes : [int]

    NONE_CHARACTER = 0
    S_CHARACTER = 1
    O_CHARACTER = 2

    def __init__(self, size):
        self.turn = 0
        self.boxes = [self.NONE_CHARACTER for _ in range(size)]

    def has_finished(self):
        return self.has_winner() or all(character != self.NONE_CHARACTER for character in self.boxes)

    def has_winner(self):
        return self.get_winner() is not None

    def get_winner(self):
        sequence_length = 0
        i = 0
        while i < len(self.boxes):
            character = self.boxes[i]
            if sequence_length == 0 and character == self.S_CHARACTER:
                sequence_length += 1
                i += 1
                continue
            if sequence_length == 1 and character == self.O_CHARACTER:
                sequence_length += 1
                i += 1
                continue
            if sequence_length == 2 and character == self.S_CHARACTER:
                return self.get_previous_player()
            i = i + 1 - sequence_length
            sequence_length = 0
        return None

    def get_previous_player(self):
        if self.turn == 0:
            raise RuntimeError("No previous turn")
        return Player((self.turn + 1) % 2)

    def get_next_player(self):
        return Player(self.turn % 2)

    def num_characters(self):
        return len(self.boxes)

    def put_character(self, character : str, index : int) -> 'GameState':
        if index not in range(len(self.boxes)) or self.boxes[index] != self.NONE_CHARACTER:
            raise RuntimeError("Cannot put character at that index")

        new_state = GameState(self.num_characters())
        new_state.turn = self.turn + 1
        new_state.boxes = list(self.boxes)

        if character == "S":
            new_state.boxes[index] = self.S_CHARACTER
        elif character == "O":
            new_state.boxes[index] = self.O_CHARACTER
        else:
            raise RuntimeError(f"Cannot put character '{character}'")
                
        return new_state

    def get_characters(self):
        result = ""
        for character in self.boxes:
            if character == self.NONE_CHARACTER:
                result += "_"
            elif character == self.S_CHARACTER:
                result += "S"
            elif character == self.O_CHARACTER:
                result += "O"
            else:
                result += "?"
        return result

    def get_free_positions(self):
        return [i for i in range(len(self.boxes)) if self.boxes[i] == self.NONE_CHARACTER]

    def get_possible_moves(self):
        if self.has_finished():
            return []
        free_positions = self.get_free_positions()
        return [("S", pos) for pos in free_positions] + [("O", pos) for pos in free_positions]

def build_state_tree(root_state : GameState):
    root = tree.AnyNode()
    root.state = root_state
    for (character, position) in root_state.get_possible_moves():
        subtree = build_state_tree(root_state.put_character(character, position))
        subtree.parent = root
        subtree.last_added_character = character
        subtree.last_written_position = position
    return root

def evaluate_tree(root : tree.AnyNode, player : Player):
    if root.state.has_finished():
        winner = root.state.get_winner()
        if winner is None:
            root.evaluation = 0.0
        elif winner == player:
            root.evaluation = 1.0
        else:
            root.evaluation = -1.0
    else:
        for child in root.children:
            evaluate_tree(child, player)
        if root.state.get_next_player() == player:
            root.evaluation = max([child.evaluation for child in root.children])
        else:
            root.evaluation = min([child.evaluation for child in root.children])

def get_best_move(game_state : GameState):
    state_tree = build_state_tree(game_state)
    evaluate_tree(state_tree, game_state.get_next_player())
    best_child = None
    top_evaluation = None
    for child in state_tree.children:
        if top_evaluation is None or child.evaluation > top_evaluation:
            top_evalution = child.evaluation
            best_child = child

    return (child.last_added_character, child.last_written_position, child.evaluation)

game_state = GameState(size=6)

state_tree = build_state_tree(game_state)
evaluate_tree(state_tree, Player.SECOND)
for pre, fill, node in tree.RenderTree(state_tree):
    print(f"{pre}{node.state.get_characters()} ({node.evaluation})")
print()
