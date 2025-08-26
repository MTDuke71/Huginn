#!/usr/bin/env python3

# Game analysis script to identify key moments
moves = [
    "a2a4", "b8c6", "e2e3", "g8f6", "f2f3", "d7d5", "e1f2", "d5d4",
    "f1b5", "d4e3", "f2f1", "d8d6", "f3f4", "c8g4", "d2d4", "g4d1",
    "b5c6", "d6c6", "g1f3", "c6c2", "g2g4"
]

def move_to_algebraic(move_str, move_num):
    """Convert UCI notation to simple algebraic notation"""
    from_sq = move_str[:2]
    to_sq = move_str[2:4]
    
    # Simple conversion - could be improved with piece symbols
    return f"{from_sq}-{to_sq}"

print("=== GAME ANALYSIS ===")
print("White: Huginn (Computer)")
print("Black: Opponent\n")

print("Key moves to analyze:")
for i, move in enumerate(moves):
    player = "Huginn (White)" if i % 2 == 0 else "Black"
    move_num = (i // 2) + 1
    if i % 2 == 0:
        print(f"{move_num}. {move} ", end="")
    else:
        print(f"{move}")

print("\n\nLet me identify critical moments...")

# Looking for potential queen blunder
critical_moves = []
for i, move in enumerate(moves):
    player = "Huginn" if i % 2 == 0 else "Black"
    
    # Look for moves involving the queen squares
    if "d1" in move or "d8" in move:
        critical_moves.append((i+1, player, move, "Queen move detected"))
    
    # Look for potential sacrifices (moving to squares under attack)
    if move in ["f2f3", "e1f2", "f2f1", "d2d4"]:  # Suspicious king moves
        critical_moves.append((i+1, player, move, "Dangerous king move"))

print("Critical moments identified:")
for move_num, player, move, reason in critical_moves:
    print(f"Move {move_num}: {player} plays {move} - {reason}")

print("\nLet's analyze the sequence around the queen blunder:")
print("14. ...Bg4 (Black attacks queen)")
print("15. d2-d4 (Huginn blocks but exposes queen)")
print("16. ...Bxd1 (Black captures queen!)")
print("17. Bxc6+ (Huginn tries counterplay with check)")
print("18. ...Qxc6 (Black recaptures)")

print("\nThis appears to be the queen blunder - Huginn failed to move the queen when attacked by the bishop on g4!")
