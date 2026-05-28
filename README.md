# · − − · Morse Code Translator

My 2nd semester DSA project — built in **C + Qt**. Morse code has always been my favourite language, so when the assignment said *pick a project*, this was the only option.

![Qt](https://img.shields.io/badge/Qt-Framework-41CD52?style=flat&logo=qt) ![C](https://img.shields.io/badge/Language-C-00599C?style=flat&logo=c) ![DSA](https://img.shields.io/badge/DSA-Binary_Search_Tree-orange?style=flat) ![Languages](https://img.shields.io/badge/Languages-11-blueviolet?style=flat)

---

## What it does

- Translates **text → Morse** and **Morse → text** in real time
- Live **dot-dash animation** where each symbol glows as it plays
- **Speed slider** — 5 levels from Very Slow to Blazing Fast
- **11 languages** — English, Hindi, Russian, Arabic, Japanese, Greek, French, Spanish, German, Portuguese, Chinese
- One-click **Swap Mode** to reverse the translation direction

## How it works (the DSA part)

The entire engine is a **Binary Search Tree**:

```
        ROOT
       /    \
     [·]    [−]
     / \    / \
    E   A  T   M
```

- `·` dot → go **left**
- `−` dash → go **right**
- The character at the final node = decoded letter
- Encoding uses a reverse O(1) lookup map
- 64 nodes covering A–Z and 0–9

Non-Latin scripts (Hindi, Russian, Arabic etc.) are handled via phonetic transliteration maps before hitting the BST.

## Run it

1. Open `MorseCodeTranslator.pro` in **Qt Creator**
2. Select a Desktop kit (MinGW or MSVC)
3. Hit **Ctrl + R**

## Stack

- Language: **C**
- UI: **Qt Widgets** — QPainter, QPropertyAnimation, QSlider
- Data Structure: **Binary Search Tree**

---

*B.Tech CSE · UPES Dehradun · Semester 2 · DSA Assignment*
