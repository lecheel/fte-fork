# Software Design Specification: Streamlined FTE (Ncurses-Only)

---

## 1. Overview & Architectural Goals

**FTE (Folding Text Editor)** is a lightweight, high-performance, model-driven text editor designed for UNIX/Linux terminal environments. Following the architectural cleanup that removed legacy graphical and platform backends (X11, Qt, Motif, OS/2 PM, Win32, and DOS), this specification defines the design of the **streamlined, ncurses-exclusive FTE**.

### Core Design Goals
* **Zero External Dependencies beyond POSIX & Ncurses**: Strict terminal-only execution with high portability.
* **Dual-Tier Buffer Architecture**: Sub-millisecond code folding via independent real-line and visual-line gap buffers.
* **Model-View-Controller (MVC) Separation**: Clear detachment of raw file models (`EModel`/`EBuffer`), visual presentation viewports (`EView`/`EEditPort`), and window management (`GFrame`/`GView`).
* **Deterministic Configuration Compilation**: Static offline configuration bytecode compilation (`cfte`) with zero-allocation internal fallback (`defcfg.h`).
* **Non-Blocking External I/O**: Integrated non-blocking pipe architecture supporting real-time external tool feedback (Git status, grep, compilers).

---

## 2. System Architecture

The architecture is divided into four cohesive, loosely-coupled layers:

```
┌────────────────────────────────────────────────────────────────────────┐
│                        Presentation Layer                              │
│  con_ncurses.cpp | menu_text.cpp | g_draw.cpp | g_text.cpp             │
├────────────────────────────────────────────────────────────────────────┤
│                     Windowing & Modal Framework                        │
│  GFrame / GView  | EFrame / GxView | ExView (i_input, i_choice, etc.)  │
├────────────────────────────────────────────────────────────────────────┤
│                       Editor Logic & Controller                        │
│  EGUI / EView    | c_bind (Keymaps & Macros) | e_search | e_undo       │
├────────────────────────────────────────────────────────────────────────┤
│                        Model & Storage Layer                           │
│  EBuffer (Gap buffers) | ELine | EFold | EDirectory | EMessages        │
├────────────────────────────────────────────────────────────────────────┤
│                        Subsystem Services                              │
│  e_regex | c_hilit | e_git | s_files | g_unix_pipe                     │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 3. Storage & Buffer Subsystem

The editor core revolves around `EBuffer`, which represents an open file or text scrap.

### 3.1. Dual Gap-Buffer Architecture
To guarantee $O(1)$ amortized insertions/deletions and $O(\log N)$ visual lookups with folding:

1. **Real Line Array (`LL`)**:
   * Contains pointers to actual line objects (`PELine *LL`).
   * Governed by a gap pointer `RGap`, count `RCount`, and capacity `RAllocated`.
   * Holds the true contents of the file on disk.

2. **Visual Line Array (`VV`)**:
   * Represents lines currently rendered on screen (skipping folded regions).
   * Maps a visual row $V$ to its real row offset: $\text{RealRow}(V) = V + VV[V]$.
   * Governed by its own gap buffer (`VGap`, `VCount`, `VAllocated`).

```
Real Array (LL):    [ L0 ] [ L1 ] [ L2 ] [ L3 ] [ L4 ] [ L5 ] [ L6 ]
                                  \____ FOLDED ____/
Visual Array (VV):  [ V0=0 ] [ V1=0 ] [ V2=+3 ] [ V3=+3 ]
Rendered Lines:        L0        L1        L5        L6
```

### 3.2. Line Storage (`ELine`)
* Represents a continuous byte stream for a single line without trailing line terminators.
* Line termination character styles (LF, CRLF, CR) are auto-detected on load (`BFI_DetectLineSep`) and preserved as buffer-wide properties (`BFI_AddCR`, `BFI_AddLF`).
* Contains `StateE`: the syntax highlighter end-state cache for $O(1)$ redraw calculation.

### 3.3. Folding Engine (`e_fold.cpp`)
* Folds are tracked in a sorted array `EFold *FF` containing `{ int line; char level; char open; }`.
* Binary search (`FindNearFold`) resolves fold positions in $O(\log F)$ time.
* Folds can be created manually, by indentation, or automatically by regular expressions (e.g., matching function definitions via `RoutineRegexp`).
* **Persistence**: Folds and bookmarks can be serialized into source comments upon saving (`BFI_SaveFolds` / `BFI_SaveBookmarks`), allowing persistent fold states without external metadata sidecars.

---

## 4. Windowing and UI Architecture

### 4.1. Frame & View Hierarchy
FTE separates the visual screen space into tiled frames and views:

* **`GFrame`**: Encapsulates a whole-screen layout. Manages the top menu bar (`menu_text.cpp`), active child views, and horizontal view splits (`ConSplitView`).
* **`GView` / `GxView`**: Represents a viewport tile on screen. Supports horizontal splitting, dynamic resizing, and scrollbar drawing.
* **`ExView`**: Modal view stack pushed onto a `GxView`:
  * `ExModelView`: Hosts regular editor content (`EView`).
  * `ExInput`: Single-line status line prompt with history and tab-completion.
  * `ExChoice`: Modal confirmation/selection dialogs.
  * `ExISearch`: Incremental search modal layer.
  * `ExComplete`: Interactive in-place word completion carousel.

### 4.2. Terminal Drawing Pipeline (`con_ncurses.cpp` & `g_draw.cpp`)
* **Cell Abstraction**: The display surface is an array of `TCell` (16-bit integer: 8-bit attribute/color + 8-bit character code).
* **Double Buffering**: Changes are written to a logical line buffer (`PCell`), checked against a shadow buffer (`SavedScreen`), and flushed efficiently to ncurses using `waddch` / `wattrset` minimizing cursor movement and escape sequences.
* **Color Translation**: Translates FTE color codes (16 foreground $\times$ 16 background) into ncurses color pairs (`ConInitColors`).

---

## 5. Event Dispatching & Command Engine

### 5.1. Input Processing Loop
The centralized event pump in `con_ncurses.cpp` and `egui.cpp`:
1. **Poll & Signal Handling**: Polls `STDIN_FILENO` and child pipe descriptors (`WaitPipeEvent`).
2. **Key Decoding**: Decodes multi-byte VT100/ANSI/xterm escape sequences into unified `TKeyCode` integers supporting `kfCtrl`, `kfAlt`, `kfShift`, and functional keys (`kbUp`, `kbF1`, etc.).
3. **Dispatch**: Routes events to the top `ExView` of the active `GView`. If unhandled, delegates to `EGUI::DispatchKey`.

```
Terminal Input ──> ConGetEvent() ──> Decode Escape/Key ──> EGUI::DispatchKey()
                                                                 │
                 ┌───────────────────────────────────────────────┘
                 ▼
          Find EventMap (Mode -> Parent Modes)
                 │
                 ├── Match? ──> NewCommand / ExecMacro()
                 │                   │
                 │                   ▼
                 │             ExecCommand(ExCommands)
                 │                   │
                 │                   ▼
                 │             Buffer Mutation & Redraw
                 │
                 └── Unmatched Printable ──> EBuffer::TypeChar()
```

### 5.2. Commands & Macros (`c_bind.cpp`)
* All editor actions are enumerated in `ExCommands` (e.g., `ExMoveUp`, `ExBlockCopy`, `ExFindReplace`).
* Mapped via names in `c_cmdtab.h` to strings like `"MoveUp"`, `"BlockCopy"`.
* Macros consist of an array of atomic command invocations, numeric parameters, string literals, and dynamic variable evaluations (`$FilePath`, `$CurRow`, `$Word`).

---

## 6. Undo/Redo Engine (`e_undo.cpp`)

The undo architecture operates on a linear tape of byte-encoded reversal transactions:

* **Transaction Chunking**: Successive insertions/deletions belong to a single undo transaction until a boundary action (`NextCommand`) commits the transaction.
* **Undo Records**:
  * `ucInsChars`: Rollback deletes the inserted character range.
  * `ucDelChars`: Rollback restores the deleted text and coordinates.
  * `ucInsLine` / `ucDelLine`: Restores line structure.
  * `ucPosition`: Reverts cursor coordinates.
  * `ucFold*`: Reverts folding structural changes.
  * `ucModified`: Tracks transition back to un-dirty file status.
* Memory is preserved using an exponentially growing contiguous storage array (`US.Data`).

---

## 7. Syntax Highlighting & Indentation (`c_hilit.cpp`, `h_*.cpp`)

Syntax highlighting operates on a line-by-line basis:
1. **State Preservation**: Highlighting functions receive `hlState &State` (the state from the end of the previous line) and return the updated state.
2. **Selective Invalidation**: When line $N$ is modified, highlighting is recalculated forward starting from line $N$. If the resulting `StateE` matches the pre-existing state of the following line, parsing terminates early ($O(1)$ editing performance in large files).
3. **Dual Parsing Engines**:
   * **Direct C++ Analyzers**: High-performance hardcoded state machines for major languages (`h_c.cpp`, `h_perl.cpp`, `h_sh.cpp`, etc.).
   * **Generic Table Machine (`HMachine` / `h_simple.cpp`)**: Configurable keyword and regex-driven tokenizer configured directly from `.fte` rule files.

---

## 8. Asynchronous Processes & Git Integration

### 8.1. Pipe Subsystem (`g_unix_pipe.cpp`)
* Manages child processes (compilers, grep, subshells) using non-blocking UNIX pipes (`pipe()`, `fork()`, `exec()`).
* Multiplexes pipes within `ConGetEvent` using POSIX `select()`.
* Delivers stdout/stderr asynchronously to model listeners (`EMessages`, `ECvsBase`) without blocking editor navigation.

### 8.2. Real-Time Git Gutter (`e_git.cpp`, `o_buffer.cpp`)
* Computes live hunk state (`GIT_ADDED`, `GIT_MODIFIED`, `GIT_DELETED`) per line by invoking:
  ```bash
  git -C "<dir>" diff --no-color -U0 HEAD -- "<file>"
  ```
* Supports unsaved in-memory buffers by executing `--no-index` diffs between temporary base commits and memory snapshots.
* Updates the gutter margin on an idle debounce timer (350–400ms) without stalling user keystrokes.

---

## 9. Configuration & Build Architecture

### 9.1. The `cfte` Configuration Compiler
FTE does not parse human-readable text configuration at runtime. Instead:
1. User configs (`*.fte`) are compiled by `cfte` into a binary format (`defcfg.cnf`).
2. `mkdefcfg.pl` converts `defcfg.cnf` into a static C byte array in `defcfg.h`.
3. If no external `~/.fterc` exists, FTE bootstraps entirely from internal compiled-in memory (`UseDefaultConfig()`).

```
   [ defcfg.fte ] ──> cfte ──> [ defcfg.cnf ] ──> mkdefcfg.pl ──> [ defcfg.h ]
                                                                       │
                                                                       ▼
                                                          Compiled into `c_config.o`
```

### 9.2. Simplified Makefile Layout
* `Makefile`: Top-level router routing `make` directly to `fte-unix.mak`.
* `objs.inc`: Unified source listing (`OBJS`, `NOBJS`, `CFTE_OBJS`).
* `fte-unix.mak`: Compiles `cfte`, produces `defcfg.h`, and links `nfte` strictly against `libncurses`.

---

## 10. Summary of Architectural Advantages

| Dimension | Legacy Multi-Platform FTE | Streamlined Ncurses FTE |
|---|---|---|
| **Code Size** | ~144 files, >46,000 LOC | ~120 files, ~35,000 LOC (-25% dead code) |
| **Dependencies** | X11, Xt, Motif, Qt3, OS/2 APIs, Win32 | Standard POSIX, libc, libncurses |
| **Display Drivers** | 6 fragmented driver files | Single unified driver (`con_ncurses.cpp`) |
| **Portability** | Requires cross-compilers/vintage SDKs | Any modern Linux/BSD/POSIX environment |
| **Maintenance** | Complex `#ifdef` matrices for OS/2 & DOS | Uniform POSIX-compliant C++ codebase |
