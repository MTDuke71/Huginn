#!/usr/bin/env python3
"""
Analysis of WAC.002 position: 8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -
"""

def analyze_position():
    print("WAC.002 Position Analysis")
    print("=" * 50)
    print("FEN: 8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1")
    print()
    
    print("Position Description:")
    print("- Material: Roughly equal (R+P vs R+P)")
    print("- Black to move")
    print("- Black rook on b3, white rook on d2") 
    print("- White has pawns on b2, f4, g2, h2")
    print("- Black has pawns on a4, c4, d3, f5, h7")
    print("- White king on f3, Black king on f6")
    print()
    
    print("Candidate Moves Analysis:")
    print("-" * 30)
    
    print("1. Rxb2 (WAC expected):")
    print("   - Captures the b2 pawn")
    print("   - Creates immediate material advantage") 
    print("   - Rook becomes active on the 2nd rank")
    print("   - Threatens back rank tactics")
    print("   - After Rxb2, White's rook is somewhat passive")
    print()
    
    print("2. Rb7 (Stockfish preference):")
    print("   - More positional approach")
    print("   - Rook goes to the 7th rank (traditionally strong)")
    print("   - Attacks h7 pawn (but it's defended)")
    print("   - Keeps pressure without immediate tactics")
    print("   - May be deeper positional understanding")
    print()
    
    print("3. c4c3 (Huginn choice):")
    print("   - Advances the passed pawn")
    print("   - Creates immediate threats")
    print("   - After bxc3, opens the b-file further")
    print("   - Practical and forcing")
    print("   - May lead to simplification")
    print()
    
    print("Why the disagreement?")
    print("-" * 20)
    print("• WAC tests often prefer immediate tactical gains")
    print("• Stockfish sees deeper positional factors")
    print("• Huginn focuses on concrete pawn advances")
    print("• Time control affects move selection")
    print("• Different evaluation criteria weight material vs position")

if __name__ == "__main__":
    analyze_position()
