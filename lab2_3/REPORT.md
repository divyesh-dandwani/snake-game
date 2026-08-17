# Lab 2_3 — Group A__

> Copy this file to `lab2_3/REPORT.md` in your fork and fill it in. Keep the eight
> headings exactly as they are — they are the marking scheme, in order. Delete this
> quote block when you are done.

---

## 1. Tool and install route — [3]

| | |
|---|---|
| Agent used for run 2 | Gemini IDE (Antigravity) |
| `ubiquitous-language` install route | Workspace `.agents/skills` |
| `refactoring/` pack install route | Workspace `.agents/skills` |

If anything would not install, say what failed here. The paste fallback is a documented
route and costs no marks.

Everything installed successfully without issues.

---

## 2. What I changed in the glossary — [4]

The generated file is at `lab2_3/UBIQUITOUS_LANGUAGE.md`. Three or four lines here on what
you corrected and why: terms it invented, definitions it got wrong, ambiguities it missed.

The AI correctly found that the domain uses "Fruit" while the code uses "Food". I kept the distinction under 'Code drift'. However, it added 'pellet' as an alias for Fruit, which nobody uses, so I removed it. It also guessed 'map' as an alias for Board, which is not applicable here, so I removed it as well.

---

## 3. Smell delta — [6]

Reports: `lab2_3/audits/main.md` (the code as you received it) and `lab2_3/audits/lab1-head.md`
(after your Lab-1 PR).

| | count | representative site (`file:line`) |
|---|---|---|
| Smells my Lab-1 PR **introduced** | 2 | `game.cpp:780` (Duplicate Code) |
| Smells my Lab-1 PR **left untouched** | 4 | `game.cpp:81` (Data Clumps) |
| Smells my Lab-1 PR **removed** | 0 | - |

The third row will probably be zero. Leave it in.

---

## 4. Rejected candidates — [6]

At least three things the agent reported that are *not* real findings on this codebase.

| smell reported | `file:line` | why it does not hold |
|---|---|---|
| Feature Envy | `game.cpp:741` | `checkCollision` inherently needs to compare the snake's position against the board and obstacles; it is the correct place for this orchestration. |
| Temporary Field | `game.cpp:350` | The `growing` field accurately models the transient state of the snake extending after eating; it is an essential game mechanic, not a flaw. |
| Long Parameter List | `game.cpp:81` | `PowerUp::spawn` takes 5 parameters, but it genuinely needs the entire board state to avoid spawning on top of existing entities. Grouping them wouldn't improve cohesion. |

---

## 5. Commit map — [7]

Run `lab2_3/check-lab2_3.sh` and paste the table it prints.

| # | sha | subject | what it is |
|---|---|---|---|
| 1 | 9fd92a2 | Part B: Add and edit ubiquitous language glossary | glossary |
| 2 | 096bdb4 | Part C: Add code smell audit for main branch | smell report |
| 3 | dd75215 | Part D: Refactor snake tracking to support multiple players | the refactor alone |
| 4 | e8f5e9c | Part D: Add second player feature | the feature alone |

---

## 6. Two-run measurement — [4]

Run 1 is your Lab-1 branch — the numbers you already reported. Run 2 is commit 4 alone.

| | Run 1 (Lab 1) | Run 2 (commit 4) |
|---|---|---|
| Smells introduced | 2 | 0 |
| Lines changed, `git diff --shortstat -w` | 344 | 1 file changed, 28 insertions(+), 10 deletions(-) |
| Lines changed, **raw** (no `-w`) | 344 | 1 file changed, 28 insertions(+), 10 deletions(-) |
| Functions reached | 10 | 2 |
| Prompts to working code | 3 | 1 |
| Wall-clock time | 60 mins | 15 mins |

Commit 3 (the refactor) on its own: 1 file changed, 65 insertions(+), 39 deletions(-) lines `-w`, 1 file changed, 101 insertions(+), 75 deletions(-) raw.

`check-lab2_3.sh` prints the four line-count numbers for run 2. Use them — they are measured
the same way for every group, which is what makes the class comparison mean anything.

---

## 7. Analysis Q1–Q2 — [5]

**Q1. Which smell did commit 3 actually fix?** Name it from your section 3 report. What was
expensive before, what does it cost now.

Commit 3 fixed the Data Clumps smell (where player state variables like `snake` and `score` were separated). Before, adding a new player required touching every function to duplicate logic for `snake2` and `score2`. Now, the cost is just updating `numPlayers` and adding an input mapping, reducing future feature cost dramatically.

**Q2. Compare commit 4 to your Lab-1 diff.** Same feature, same codebase. What changed in
the cost and what did not? If it got worse, say so and explain — that marks the same.

Commit 4 was tiny (38 lines vs 344 in Lab-1). The cost of implementing the feature became almost trivial because the design naturally supported multiple players through loops. However, the upfront cost shifted to the refactoring phase (Commit 3), which took planning but eliminated duplicate logic entirely.

---

## 8. Analysis Q3–Q4 — [5]

**Q3. Go back through your Lab-1 `LLM-LOG.md`. Did the assistant ever suggest restructuring
before adding the feature?** Quote it if it did. If it did not, what would have had to be
different in your prompt?

In Lab 1, the assistant did not suggest restructuring first. It simply added `snake2` and duplicated logic to satisfy the immediate feature requirement. To prompt it to restructure, we would have had to explicitly ask it to "evaluate if the design supports this feature cleanly, and refactor the architecture if not" before adding the new feature.

**Q4. How do you know commit 3 did not change behaviour?** Answer honestly. Most of you will
find that you do not know. Say that plainly if it is true, and describe what you would have
needed in order to actually know.

Honestly, I do not know for sure. Without automated integration or unit tests covering collision rules, powerups, and rendering, we are relying purely on a brief visual check. A robust suite of unit tests proving state consistency before and after would be needed to truly know behavior held.

---

Sections 7 and 8 together: **500 words maximum.**

## If you did not finish

Finished completely.
