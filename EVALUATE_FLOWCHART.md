# Evaluation Function Flow Chart

```mermaid
flowchart TD
  A[Start evaluate Position] --> B{Material Draw?}
  B -- Yes --> Z[Return 0 Draw]
  B -- No --> C[Initialize score = 0]
  C --> D[Get total material, is_endgame]
  D --> E[Loop: For each board square]
  E --> F{Piece present?}
  F -- No --> E
  F -- Yes --> G[Get piece type, color, sq64]
  G --> H[Material value by type]
  H --> I[Piece-square table value]
  I --> J[score += material + PST white, score -= PST black]
  J --> E
  E --> K[Pawn Structure Evaluation]
  K --> L[Get white/black pawn bitboards]
  L --> M[Loop: For each sq64]
  M --> N{White pawn?}
  N -- Yes --> O[Isolated pawn? Subtract penalty]
  O --> P[Passed pawn? Add bonus]
  P --> M
  N -- No --> Q{Black pawn?}
  Q -- Yes --> R[Isolated pawn? Add penalty]
  R --> S[Passed pawn? Subtract bonus]
  S --> M
  Q -- No --> M
  M --> T[score += pawn structure score]
  T --> U[Open/Semi-Open File Bonuses]
  U --> V[Loop: For each sq]
  V --> W{Rook/Queen present?}
  W -- No --> V
  W -- Yes --> X{White/Black?}
  X -- Yes --> Y[Open file? Add bonus; Semi-open? Add bonus]
  X -- No --> AA[Open file? Subtract bonus; Semi-open? Subtract bonus]
  Y --> V
  AA --> V
  V --> AB[score += file bonus score]
  AB --> AC[Bishop Pair Bonus]
  AC --> AD{White bishops >= 2?}
  AD -- Yes --> AE[Add bonus]
  AD -- No --> AF
  AE --> AF{Black bishops >= 2?}
  AF -- Yes --> AG[Subtract bonus]
  AF -- No --> AH
  AG --> AH[Return score white to move or -score black to move]
```

---

**How to view this flow chart:**

- The file uses [Mermaid](https://mermaid-js.github.io/) syntax, which is supported by many Markdown viewers and editors.
- **Best options:**
  - **VS Code**: Install the "Markdown Preview Mermaid Support" extension, then open the file and use the built-in Markdown preview (Ctrl+Shift+V).
  - **GitHub**: Paste the content into a GitHub README or Gist—GitHub renders Mermaid diagrams natively.
  - **Mermaid Live Editor**: Copy and paste the code into [Mermaid Live Editor](https://mermaid.live/).
  - **Obsidian**: Supports Mermaid diagrams out of the box.

You can also use any Markdown tool that supports Mermaid diagrams for best results.
