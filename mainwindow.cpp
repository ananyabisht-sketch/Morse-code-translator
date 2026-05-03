#include "mainwindow.h"
#include <QApplication>
#include <QClipboard>
#include <QScrollBar>
#include <QTextCharFormat>
#include <cctype>

/* ═══════════════════════════════════════════════════════════════════
   MORSE TABLE  –  International ITU standard A-Z, 0-9, punctuation
   ═══════════════════════════════════════════════════════════════════ */
static const struct { const char *code; char ch; } MORSE_TABLE[] = {
    /* Letters */
    {".-",   'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..",  'D'},
    {".",    'E'}, {"..-.", 'F'}, {"--.",  'G'}, {"....", 'H'},
    {"..",   'I'}, {".---", 'J'}, {"-.-",  'K'}, {".-..", 'L'},
    {"--",   'M'}, {"-.",   'N'}, {"---",  'O'}, {".--.", 'P'},
    {"--.-", 'Q'}, {".-.",  'R'}, {"...",  'S'}, {"-",    'T'},
    {"..-",  'U'}, {"...-", 'V'}, {".--",  'W'}, {"-..-", 'X'},
    {"-.--", 'Y'}, {"--..", 'Z'},
    /* Digits */
    {".----", '1'}, {"..---", '2'}, {"...--", '3'}, {"....-", '4'},
    {".....", '5'}, {"-....", '6'}, {"--...", '7'}, {"---..", '8'},
    {"----.", '9'}, {"-----", '0'},
    /* Punctuation */
    {".-.-.-", '.'}, {"--..--", ','}, {"..--..", '?'},
    {".----.", '\''}, {"-.-.--", '!'}, {"-..-.", '/'},
    {"-.--.",  '('}, {"-.--.-", ')'}, {".-...",  '&'},
    {"---...", ':'}, {"-.-.-.", ';'},
    {"-...-",  '='}, {".-.-.",  '+'}, {"-....-", '-'},
    {"..--.-", '_'}, {".-..-.", '"'},
    {"...-..-",'$'}, {".--.-.", '@'},
    /* End sentinel */
    {nullptr, '\0'}
};

/* ═══════════════════════════════════════════════════════════════════
   MorseBST
   ═══════════════════════════════════════════════════════════════════ */
MorseBST::MorseBST() {
    root = new MorseNode(QChar('\0'));
    for (int i = 0; MORSE_TABLE[i].code; i++) {
        QChar c = QChar::fromLatin1(MORSE_TABLE[i].ch);
        QString code = MORSE_TABLE[i].code;
        insert(code, c);
        charToMorse[c] = code;
        /* also map lowercase */
        charToMorse[QChar(MORSE_TABLE[i].ch | 0x20)] = code;
    }
    initMaps();
}

MorseBST::~MorseBST() { freeTree(root); }

void MorseBST::freeTree(MorseNode *n) {
    if (!n) return;
    freeTree(n->dot); freeTree(n->dash);
    delete n;
}

void MorseBST::insert(const QString &code, QChar ch) {
    MorseNode *cur = root;
    for (QChar c : code) {
        if (c == '.') {
            if (!cur->dot)  cur->dot  = new MorseNode(QChar('\0'));
            cur = cur->dot;
        } else {
            if (!cur->dash) cur->dash = new MorseNode(QChar('\0'));
            cur = cur->dash;
        }
    }
    cur->ch = ch;
}

QChar MorseBST::decodeSymbol(const QString &sym) const {
    MorseNode *cur = root;
    for (QChar c : sym) {
        if (!cur) return '?';
        cur = (c == '.') ? cur->dot : cur->dash;
    }
    return (cur && !cur->ch.isNull()) ? cur->ch.toUpper() : QChar('?');
}

QString MorseBST::encodeChar(QChar c) const {
    QChar upper = c.toUpper();
    if (charToMorse.contains(upper)) return charToMorse[upper];
    if (charToMorse.contains(c))     return charToMorse[c];
    return QString();
}

QString MorseBST::encodeText(const QString &text) const {
    QString result;
    for (QChar c : text) {
        if (c == ' ' || c == '\n') {
            if (!result.isEmpty() && !result.endsWith("   "))
                result += "   ";          /* 3 spaces = word gap */
        } else {
            QString code = encodeChar(c);
            if (!code.isEmpty()) {
                if (!result.isEmpty() && !result.endsWith("   "))
                    result += " ";
                result += code;
            }
        }
    }
    return result.trimmed();
}

QString MorseBST::decodeText(const QString &morse) const {
    QString result;
    QStringList words = morse.split("   ");
    for (int w = 0; w < words.size(); w++) {
        if (w > 0) result += ' ';
        QStringList syms = words[w].trimmed().split(' ', Qt::SkipEmptyParts);
        for (const QString &s : syms) {
            QChar decoded = decodeSymbol(s);
            result += decoded;
        }
    }
    return result;
}

QString MorseBST::transliterate(const QString &text,
                                 const QMap<QChar,QChar> &map) const {
    QString result;
    for (QChar c : text) {
        result += map.value(c, c);   /* keep original if no mapping */
    }
    return result;
}

/* ── Transliteration maps ── */
void MorseBST::initMaps() {
    /* ── Hindi (Devanagari) – maps common vowels/consonants to Latin ── */
    /* Vowels */
    hindiMap[QChar(0x0905)] = 'A';  /* अ */
    hindiMap[QChar(0x0906)] = 'A';  /* आ */
    hindiMap[QChar(0x0907)] = 'I';  /* इ */
    hindiMap[QChar(0x0908)] = 'I';  /* ई */
    hindiMap[QChar(0x0909)] = 'U';  /* उ */
    hindiMap[QChar(0x090A)] = 'U';  /* ऊ */
    hindiMap[QChar(0x090F)] = 'E';  /* ए */
    hindiMap[QChar(0x0913)] = 'O';  /* ओ */
    /* Consonants */
    hindiMap[QChar(0x0915)] = 'K';  /* क */
    hindiMap[QChar(0x0916)] = 'K';  /* ख */
    hindiMap[QChar(0x0917)] = 'G';  /* ग */
    hindiMap[QChar(0x0918)] = 'G';  /* घ */
    hindiMap[QChar(0x091A)] = 'C';  /* च */
    hindiMap[QChar(0x091C)] = 'J';  /* ज */
    hindiMap[QChar(0x091F)] = 'T';  /* ट */
    hindiMap[QChar(0x0921)] = 'D';  /* ड */
    hindiMap[QChar(0x0928)] = 'N';  /* न */
    hindiMap[QChar(0x092A)] = 'P';  /* प */
    hindiMap[QChar(0x092C)] = 'B';  /* ब */
    hindiMap[QChar(0x092E)] = 'M';  /* म */
    hindiMap[QChar(0x092F)] = 'Y';  /* य */
    hindiMap[QChar(0x0930)] = 'R';  /* र */
    hindiMap[QChar(0x0932)] = 'L';  /* ल */
    hindiMap[QChar(0x0935)] = 'V';  /* व */
    hindiMap[QChar(0x0938)] = 'S';  /* स */
    hindiMap[QChar(0x0939)] = 'H';  /* ह */
    hindiMap[QChar(0x0924)] = 'T';  /* त */
    hindiMap[QChar(0x0926)] = 'D';  /* द */
    hindiMap[QChar(0x092B)] = 'F';  /* फ */
    hindiMap[QChar(0x0936)] = 'S';  /* श */
    hindiMap[QChar(0x0916)] = 'K';  /* ख */
    hindiMap[QChar(0x0920)] = 'T';  /* ठ */
    /* Matras (vowel marks) */
    hindiMap[QChar(0x093E)] = 'A';  /* ा */
    hindiMap[QChar(0x093F)] = 'I';  /* ि */
    hindiMap[QChar(0x0940)] = 'I';  /* ी */
    hindiMap[QChar(0x0941)] = 'U';  /* ु */
    hindiMap[QChar(0x0942)] = 'U';  /* ू */
    hindiMap[QChar(0x0947)] = 'E';  /* े */
    hindiMap[QChar(0x094B)] = 'O';  /* ो */

    /* ── French accented characters ── */
    frenchMap[QChar(0x00E0)] = 'A';  /* à */
    frenchMap[QChar(0x00E2)] = 'A';  /* â */
    frenchMap[QChar(0x00E4)] = 'A';  /* ä */
    frenchMap[QChar(0x00E6)] = 'A';  /* æ */
    frenchMap[QChar(0x00E7)] = 'C';  /* ç */
    frenchMap[QChar(0x00E8)] = 'E';  /* è */
    frenchMap[QChar(0x00E9)] = 'E';  /* é */
    frenchMap[QChar(0x00EA)] = 'E';  /* ê */
    frenchMap[QChar(0x00EB)] = 'E';  /* ë */
    frenchMap[QChar(0x00EE)] = 'I';  /* î */
    frenchMap[QChar(0x00EF)] = 'I';  /* ï */
    frenchMap[QChar(0x00F4)] = 'O';  /* ô */
    frenchMap[QChar(0x0153)] = 'O';  /* œ */
    frenchMap[QChar(0x00F9)] = 'U';  /* ù */
    frenchMap[QChar(0x00FB)] = 'U';  /* û */
    frenchMap[QChar(0x00FC)] = 'U';  /* ü */
    frenchMap[QChar(0x00FF)] = 'Y';  /* ÿ */
    /* uppercase */
    frenchMap[QChar(0x00C0)] = 'A';  frenchMap[QChar(0x00C2)] = 'A';
    frenchMap[QChar(0x00C4)] = 'A';  frenchMap[QChar(0x00C6)] = 'A';
    frenchMap[QChar(0x00C7)] = 'C';  frenchMap[QChar(0x00C8)] = 'E';
    frenchMap[QChar(0x00C9)] = 'E';  frenchMap[QChar(0x00CA)] = 'E';
    frenchMap[QChar(0x00CE)] = 'I';  frenchMap[QChar(0x00D4)] = 'O';
    frenchMap[QChar(0x00D9)] = 'U';  frenchMap[QChar(0x00DB)] = 'U';
    frenchMap[QChar(0x00DC)] = 'U';

    /* ── Spanish ── */
    spanishMap[QChar(0x00E1)] = 'A';  /* á */
    spanishMap[QChar(0x00E9)] = 'E';  /* é */
    spanishMap[QChar(0x00ED)] = 'I';  /* í */
    spanishMap[QChar(0x00F3)] = 'O';  /* ó */
    spanishMap[QChar(0x00FA)] = 'U';  /* ú */
    spanishMap[QChar(0x00F1)] = 'N';  /* ñ */
    spanishMap[QChar(0x00FC)] = 'U';  /* ü */
    spanishMap[QChar(0x00C1)] = 'A';  spanishMap[QChar(0x00C9)] = 'E';
    spanishMap[QChar(0x00CD)] = 'I';  spanishMap[QChar(0x00D3)] = 'O';
    spanishMap[QChar(0x00DA)] = 'U';  spanishMap[QChar(0x00D1)] = 'N';

    /* ── German ── */
    germanMap[QChar(0x00E4)] = 'A';  /* ä → ae */
    germanMap[QChar(0x00F6)] = 'O';  /* ö → oe */
    germanMap[QChar(0x00FC)] = 'U';  /* ü → ue */
    germanMap[QChar(0x00DF)] = 'S';  /* ß → ss */
    germanMap[QChar(0x00C4)] = 'A';  germanMap[QChar(0x00D6)] = 'O';
    germanMap[QChar(0x00DC)] = 'U';

    /* ── Portuguese ── */
    portugueseMap[QChar(0x00E3)] = 'A';  /* ã */
    portugueseMap[QChar(0x00E1)] = 'A';  /* á */
    portugueseMap[QChar(0x00E2)] = 'A';  /* â */
    portugueseMap[QChar(0x00E7)] = 'C';  /* ç */
    portugueseMap[QChar(0x00EA)] = 'E';  /* ê */
    portugueseMap[QChar(0x00E9)] = 'E';  /* é */
    portugueseMap[QChar(0x00ED)] = 'I';  /* í */
    portugueseMap[QChar(0x00F5)] = 'O';  /* õ */
    portugueseMap[QChar(0x00F3)] = 'O';  /* ó */
    portugueseMap[QChar(0x00FA)] = 'U';  /* ú */
    portugueseMap[QChar(0x00C3)] = 'A';  portugueseMap[QChar(0x00C7)] = 'C';
    portugueseMap[QChar(0x00CA)] = 'E';  portugueseMap[QChar(0x00D5)] = 'O';

    /* ── Russian (Cyrillic) ── */
    russianMap[QChar(0x0410)] = 'A';  /* А */
    russianMap[QChar(0x0411)] = 'B';  /* Б */
    russianMap[QChar(0x0412)] = 'V';  /* В */
    russianMap[QChar(0x0413)] = 'G';  /* Г */
    russianMap[QChar(0x0414)] = 'D';  /* Д */
    russianMap[QChar(0x0415)] = 'E';  /* Е */
    russianMap[QChar(0x0416)] = 'J';  /* Ж */
    russianMap[QChar(0x0417)] = 'Z';  /* З */
    russianMap[QChar(0x0418)] = 'I';  /* И */
    russianMap[QChar(0x041A)] = 'K';  /* К */
    russianMap[QChar(0x041B)] = 'L';  /* Л */
    russianMap[QChar(0x041C)] = 'M';  /* М */
    russianMap[QChar(0x041D)] = 'N';  /* Н */
    russianMap[QChar(0x041E)] = 'O';  /* О */
    russianMap[QChar(0x041F)] = 'P';  /* П */
    russianMap[QChar(0x0420)] = 'R';  /* Р */
    russianMap[QChar(0x0421)] = 'S';  /* С */
    russianMap[QChar(0x0422)] = 'T';  /* Т */
    russianMap[QChar(0x0423)] = 'U';  /* У */
    russianMap[QChar(0x0424)] = 'F';  /* Ф */
    russianMap[QChar(0x0425)] = 'H';  /* Х */
    russianMap[QChar(0x0427)] = 'C';  /* Ч */
    russianMap[QChar(0x0428)] = 'S';  /* Ш */
    russianMap[QChar(0x042A)] = 'I';  /* Ъ */
    russianMap[QChar(0x042B)] = 'Y';  /* Ы */
    russianMap[QChar(0x042C)] = 'I';  /* Ь */
    russianMap[QChar(0x042D)] = 'E';  /* Э */
    russianMap[QChar(0x042E)] = 'U';  /* Ю */
    russianMap[QChar(0x042F)] = 'Y';  /* Я */
    /* lowercase */
    for (int i = 0x430; i <= 0x44F; i++) {
        QChar lower(i), upper(i - 0x20);
        if (russianMap.contains(upper))
            russianMap[lower] = russianMap[upper];
    }

    /* ── Greek ── */
    greekMap[QChar(0x0391)] = 'A';  greekMap[QChar(0x03B1)] = 'A';  /* Α α */
    greekMap[QChar(0x0392)] = 'B';  greekMap[QChar(0x03B2)] = 'B';  /* Β β */
    greekMap[QChar(0x0393)] = 'G';  greekMap[QChar(0x03B3)] = 'G';  /* Γ γ */
    greekMap[QChar(0x0394)] = 'D';  greekMap[QChar(0x03B4)] = 'D';  /* Δ δ */
    greekMap[QChar(0x0395)] = 'E';  greekMap[QChar(0x03B5)] = 'E';  /* Ε ε */
    greekMap[QChar(0x0396)] = 'Z';  greekMap[QChar(0x03B6)] = 'Z';  /* Ζ ζ */
    greekMap[QChar(0x0397)] = 'H';  greekMap[QChar(0x03B7)] = 'H';  /* Η η */
    greekMap[QChar(0x0398)] = 'T';  greekMap[QChar(0x03B8)] = 'T';  /* Θ θ */
    greekMap[QChar(0x0399)] = 'I';  greekMap[QChar(0x03B9)] = 'I';  /* Ι ι */
    greekMap[QChar(0x039A)] = 'K';  greekMap[QChar(0x03BA)] = 'K';  /* Κ κ */
    greekMap[QChar(0x039B)] = 'L';  greekMap[QChar(0x03BB)] = 'L';  /* Λ λ */
    greekMap[QChar(0x039C)] = 'M';  greekMap[QChar(0x03BC)] = 'M';  /* Μ μ */
    greekMap[QChar(0x039D)] = 'N';  greekMap[QChar(0x03BD)] = 'N';  /* Ν ν */
    greekMap[QChar(0x039E)] = 'X';  greekMap[QChar(0x03BE)] = 'X';  /* Ξ ξ */
    greekMap[QChar(0x039F)] = 'O';  greekMap[QChar(0x03BF)] = 'O';  /* Ο ο */
    greekMap[QChar(0x03A0)] = 'P';  greekMap[QChar(0x03C0)] = 'P';  /* Π π */
    greekMap[QChar(0x03A1)] = 'R';  greekMap[QChar(0x03C1)] = 'R';  /* Ρ ρ */
    greekMap[QChar(0x03A3)] = 'S';  greekMap[QChar(0x03C3)] = 'S';  /* Σ σ */
    greekMap[QChar(0x03A4)] = 'T';  greekMap[QChar(0x03C4)] = 'T';  /* Τ τ */
    greekMap[QChar(0x03A5)] = 'U';  greekMap[QChar(0x03C5)] = 'U';  /* Υ υ */
    greekMap[QChar(0x03A6)] = 'F';  greekMap[QChar(0x03C6)] = 'F';  /* Φ φ */
    greekMap[QChar(0x03A7)] = 'C';  greekMap[QChar(0x03C7)] = 'C';  /* Χ χ */
    greekMap[QChar(0x03A8)] = 'P';  greekMap[QChar(0x03C8)] = 'P';  /* Ψ ψ */
    greekMap[QChar(0x03A9)] = 'O';  greekMap[QChar(0x03C9)] = 'O';  /* Ω ω */

    /* ── Arabic ── */
    arabicMap[QChar(0x0627)] = 'A';  /* ا */
    arabicMap[QChar(0x0628)] = 'B';  /* ب */
    arabicMap[QChar(0x062A)] = 'T';  /* ت */
    arabicMap[QChar(0x062B)] = 'T';  /* ث */
    arabicMap[QChar(0x062C)] = 'J';  /* ج */
    arabicMap[QChar(0x062D)] = 'H';  /* ح */
    arabicMap[QChar(0x062E)] = 'K';  /* خ */
    arabicMap[QChar(0x062F)] = 'D';  /* د */
    arabicMap[QChar(0x0630)] = 'Z';  /* ذ */
    arabicMap[QChar(0x0631)] = 'R';  /* ر */
    arabicMap[QChar(0x0632)] = 'Z';  /* ز */
    arabicMap[QChar(0x0633)] = 'S';  /* س */
    arabicMap[QChar(0x0634)] = 'S';  /* ش */
    arabicMap[QChar(0x0635)] = 'S';  /* ص */
    arabicMap[QChar(0x0636)] = 'D';  /* ض */
    arabicMap[QChar(0x0637)] = 'T';  /* ط */
    arabicMap[QChar(0x0638)] = 'Z';  /* ظ */
    arabicMap[QChar(0x063A)] = 'G';  /* غ */
    arabicMap[QChar(0x0641)] = 'F';  /* ف */
    arabicMap[QChar(0x0642)] = 'Q';  /* ق */
    arabicMap[QChar(0x0643)] = 'K';  /* ك */
    arabicMap[QChar(0x0644)] = 'L';  /* ل */
    arabicMap[QChar(0x0645)] = 'M';  /* م */
    arabicMap[QChar(0x0646)] = 'N';  /* ن */
    arabicMap[QChar(0x0647)] = 'H';  /* ه */
    arabicMap[QChar(0x0648)] = 'W';  /* و */
    arabicMap[QChar(0x064A)] = 'Y';  /* ي */

    /* ── Japanese (Katakana basic set) ── */
    japaneseMap[QChar(0x30A2)] = 'A';  /* ア */
    japaneseMap[QChar(0x30A4)] = 'I';  /* イ */
    japaneseMap[QChar(0x30A6)] = 'U';  /* ウ */
    japaneseMap[QChar(0x30A8)] = 'E';  /* エ */
    japaneseMap[QChar(0x30AA)] = 'O';  /* オ */
    japaneseMap[QChar(0x30AB)] = 'K';  /* カ */
    japaneseMap[QChar(0x30AD)] = 'K';  /* キ */
    japaneseMap[QChar(0x30AF)] = 'K';  /* ク */
    japaneseMap[QChar(0x30B1)] = 'K';  /* ケ */
    japaneseMap[QChar(0x30B3)] = 'K';  /* コ */
    japaneseMap[QChar(0x30B5)] = 'S';  /* サ */
    japaneseMap[QChar(0x30B7)] = 'S';  /* シ */
    japaneseMap[QChar(0x30B9)] = 'S';  /* ス */
    japaneseMap[QChar(0x30BB)] = 'S';  /* セ */
    japaneseMap[QChar(0x30BD)] = 'S';  /* ソ */
    japaneseMap[QChar(0x30BF)] = 'T';  /* タ */
    japaneseMap[QChar(0x30C1)] = 'C';  /* チ */
    japaneseMap[QChar(0x30C4)] = 'T';  /* ツ */
    japaneseMap[QChar(0x30C6)] = 'T';  /* テ */
    japaneseMap[QChar(0x30C8)] = 'T';  /* ト */
    japaneseMap[QChar(0x30CA)] = 'N';  /* ナ */
    japaneseMap[QChar(0x30CB)] = 'N';  /* ニ */
    japaneseMap[QChar(0x30CC)] = 'N';  /* ヌ */
    japaneseMap[QChar(0x30CD)] = 'N';  /* ネ */
    japaneseMap[QChar(0x30CE)] = 'N';  /* ノ */
    japaneseMap[QChar(0x30CF)] = 'H';  /* ハ */
    japaneseMap[QChar(0x30D2)] = 'H';  /* ヒ */
    japaneseMap[QChar(0x30D5)] = 'F';  /* フ */
    japaneseMap[QChar(0x30D8)] = 'H';  /* ヘ */
    japaneseMap[QChar(0x30DB)] = 'H';  /* ホ */
    japaneseMap[QChar(0x30DE)] = 'M';  /* マ */
    japaneseMap[QChar(0x30DF)] = 'M';  /* ミ */
    japaneseMap[QChar(0x30E0)] = 'M';  /* ム */
    japaneseMap[QChar(0x30E1)] = 'M';  /* メ */
    japaneseMap[QChar(0x30E2)] = 'M';  /* モ */
    japaneseMap[QChar(0x30E4)] = 'Y';  /* ヤ */
    japaneseMap[QChar(0x30E6)] = 'Y';  /* ユ */
    japaneseMap[QChar(0x30E8)] = 'Y';  /* ヨ */
    japaneseMap[QChar(0x30E9)] = 'R';  /* ラ */
    japaneseMap[QChar(0x30EA)] = 'R';  /* リ */
    japaneseMap[QChar(0x30EB)] = 'R';  /* ル */
    japaneseMap[QChar(0x30EC)] = 'R';  /* レ */
    japaneseMap[QChar(0x30ED)] = 'R';  /* ロ */
    japaneseMap[QChar(0x30EF)] = 'W';  /* ワ */
    japaneseMap[QChar(0x30F3)] = 'N';  /* ン */

    /* Hiragana mirror */
    for (int k = 0x3041; k <= 0x3096; k++) {
        QChar hira(k), kata(k + 0x60);
        if (japaneseMap.contains(kata))
            japaneseMap[hira] = japaneseMap[kata];
    }

    /* ── Chinese (Pinyin approximation of common chars) ── */
    chineseMap[QChar(0x4E2D)] = 'Z';  /* 中 */
    chineseMap[QChar(0x6587)] = 'W';  /* 文 */
    chineseMap[QChar(0x4E00)] = 'Y';  /* 一 */
    chineseMap[QChar(0x4EBA)] = 'R';  /* 人 */
    chineseMap[QChar(0x5927)] = 'D';  /* 大 */
    chineseMap[QChar(0x5C0F)] = 'X';  /* 小 */
    chineseMap[QChar(0x4E0A)] = 'S';  /* 上 */
    chineseMap[QChar(0x4E0B)] = 'X';  /* 下 */
    chineseMap[QChar(0x5DE5)] = 'G';  /* 工 */
    chineseMap[QChar(0x5F00)] = 'K';  /* 开 */
    chineseMap[QChar(0x5173)] = 'G';  /* 关 */
    chineseMap[QChar(0x597D)] = 'H';  /* 好 */
    chineseMap[QChar(0x6211)] = 'W';  /* 我 */
    chineseMap[QChar(0x4F60)] = 'N';  /* 你 */
    chineseMap[QChar(0x4ED6)] = 'T';  /* 他 */
    chineseMap[QChar(0x5979)] = 'T';  /* 她 */
    chineseMap[QChar(0x4EEC)] = 'M';  /* 们 */
    chineseMap[QChar(0x662F)] = 'S';  /* 是 */
    chineseMap[QChar(0x7684)] = 'D';  /* 的 */
    chineseMap[QChar(0x5728)] = 'Z';  /* 在 */
    chineseMap[QChar(0x6709)] = 'Y';  /* 有 */
    chineseMap[QChar(0x548C)] = 'H';  /* 和 */
    chineseMap[QChar(0x4E0D)] = 'B';  /* 不 */
    chineseMap[QChar(0x8FD9)] = 'Z';  /* 这 */
    chineseMap[QChar(0x90A3)] = 'N';  /* 那 */
}

/* ═══════════════════════════════════════════════════════════════════
   GlowDot
   ═══════════════════════════════════════════════════════════════════ */
GlowDot::GlowDot(bool isDash, QWidget *p)
    : QWidget(p), m_isDash(isDash), m_bright(0)
{
    setFixedSize(isDash ? 36 : 16, 16);
    m_anim = new QPropertyAnimation(this, "brightness", this);
    m_anim->setDuration(500);
    m_anim->setEasingCurve(QEasingCurve::OutCubic);
}

void GlowDot::flash() {
    m_anim->stop();
    setBrightness(255);
    m_anim->setStartValue(255);
    m_anim->setEndValue(40);
    m_anim->start();
}

void GlowDot::dim() {
    m_anim->stop();
    setBrightness(40);
}

void GlowDot::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    /* base glow color: cyan */
    int r = 0   + (0   - 0)   * m_bright / 255;
    int g = 40  + (255 - 40)  * m_bright / 255;
    int b = 60  + (255 - 60)  * m_bright / 255;
    QColor col(r, g, b);

    if (m_bright > 80) {
        /* outer glow */
        QColor glow(0, 255, 255, m_bright / 4);
        p.setPen(Qt::NoPen);
        p.setBrush(glow);
        if (m_isDash)
            p.drawRoundedRect(-3, -3, width()+6, height()+6, 8, 8);
        else
            p.drawEllipse(-3, -3, 22, 22);
    }

    p.setBrush(col);
    p.setPen(QPen(col.lighter(130), 1));

    if (m_isDash)
        p.drawRoundedRect(0, 4, width(), 8, 4, 4);
    else
        p.drawEllipse(1, 1, 14, 14);
}

/* ═══════════════════════════════════════════════════════════════════
   MorseStrip
   ═══════════════════════════════════════════════════════════════════ */
MorseStrip::MorseStrip(QWidget *p) : QWidget(p) {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(12, 6, 12, 6);
    m_layout->setSpacing(5);
    m_layout->addStretch();
    setMinimumHeight(34);
    setStyleSheet("background:transparent;");
}

void MorseStrip::loadMorse(const QString &morse) {
    for (GlowDot *d : m_dots) { m_layout->removeWidget(d); delete d; }
    m_dots.clear();
    delete m_layout->takeAt(0); /* remove stretch */

    for (QChar c : morse) {
        if (c == '.') {
            auto *d = new GlowDot(false, this);
            m_dots << d;
            m_layout->addWidget(d);
        } else if (c == '-') {
            auto *d = new GlowDot(true, this);
            m_dots << d;
            m_layout->addWidget(d);
        } else {
            QWidget *sp = new QWidget(this);
            sp->setFixedSize(c == ' ' ? 10 : 0, 1);
            m_layout->addWidget(sp);
        }
    }
    m_layout->addStretch();
}

void MorseStrip::flashAt(int idx) {
    if (idx >= 0 && idx < m_dots.size())
        m_dots[idx]->flash();
}

void MorseStrip::resetAll() {
    for (GlowDot *d : m_dots) d->dim();
}

/* ═══════════════════════════════════════════════════════════════════
   MAINWINDOW – Language list
   ═══════════════════════════════════════════════════════════════════ */
void MainWindow::initLangs() {
    m_langs = {
        {"English (A-Z)",        "🇬🇧", "Latin",      "Direct encoding – full ITU Morse support"},
        {"Hindi (हिंदी)",          "🇮🇳", "Devanagari", "Devanagari → Latin phonetic transliteration"},
        {"French (Français)",    "🇫🇷", "Latin+",     "Accented chars (é,è,ç…) mapped to base letters"},
        {"Spanish (Español)",    "🇪🇸", "Latin+",     "Accented chars (á,ñ,ü…) mapped to base letters"},
        {"German (Deutsch)",     "🇩🇪", "Latin+",     "Umlauts (ä,ö,ü,ß) mapped to base equivalents"},
        {"Portuguese",           "🇧🇷", "Latin+",     "Accented chars (ã,ç,õ…) mapped to base letters"},
        {"Russian (Русский)",    "🇷🇺", "Cyrillic",   "Cyrillic → Latin phonetic transliteration"},
        {"Greek (Ελληνικά)",     "🇬🇷", "Greek",      "Greek alphabet → Latin phonetic mapping"},
        {"Arabic (العربية)",      "🇸🇦", "Arabic",     "Arabic consonants → Latin phonetic mapping"},
        {"Japanese (日本語)",     "🇯🇵", "Kana",       "Katakana/Hiragana → Latin phonetic mapping"},
        {"Chinese (中文)",        "🇨🇳", "CJK",        "Common CJK characters → Pinyin initial mapping"},
    };
}

/* ═══════════════════════════════════════════════════════════════════
   MAINWINDOW – Build UI
   ═══════════════════════════════════════════════════════════════════ */
MainWindow::MainWindow(QWidget *p)
    : QMainWindow(p), m_toMorse(true), m_playIdx(0)
{
    m_tree  = new MorseBST();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTick);
    initLangs();
    buildUI();
    applyTheme();
    setWindowTitle("Morse Code Translator  ·  DSA Project");
    setMinimumSize(1000, 740);
    resize(1100, 800);
}

MainWindow::~MainWindow() { delete m_tree; }

/* ── helper: section frame ── */
QFrame *MainWindow::makeSection(const QString &title, QLayout *content) {
    QFrame *f = new QFrame;
    f->setObjectName("section");
    QVBoxLayout *vl = new QVBoxLayout(f);
    vl->setContentsMargins(16, 12, 16, 14);
    vl->setSpacing(10);
    if (!title.isEmpty()) {
        QLabel *lbl = new QLabel(title);
        lbl->setObjectName("sectionTitle");
        vl->addWidget(lbl);
    }
    vl->addLayout(content);
    return f;
}

void MainWindow::buildUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    /* ════════════ HEADER ════════════ */
    QWidget *header = new QWidget;
    header->setObjectName("header");
    header->setFixedHeight(90);
    QHBoxLayout *hl = new QHBoxLayout(header);
    hl->setContentsMargins(28, 0, 28, 0);

    /* Left: title block */
    QVBoxLayout *titleCol = new QVBoxLayout;
    QLabel *appName = new QLabel("· − −  MORSE TRANSLATOR  − − ·");
    appName->setObjectName("appName");
    QLabel *appSub = new QLabel("Binary Search Tree · Multi-Language · ITU Standard");
    appSub->setObjectName("appSub");
    titleCol->addWidget(appName);
    titleCol->addWidget(appSub);
    hl->addLayout(titleCol);
    hl->addStretch();

    /* Right: mode pill + DSA badge */
    m_modeLabel = new QLabel("TEXT  →  MORSE");
    m_modeLabel->setObjectName("modePill");

    QLabel *dsa = new QLabel("BST · O(n) encode · O(m) decode");
    dsa->setObjectName("dsaBadge");

    QVBoxLayout *rightCol = new QVBoxLayout;
    rightCol->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rightCol->addWidget(m_modeLabel);
    rightCol->addWidget(dsa);
    hl->addLayout(rightCol);

    root->addWidget(header);

    /* thin accent line */
    m_glowLine = new QFrame;
    m_glowLine->setObjectName("accentLine");
    m_glowLine->setFixedHeight(2);
    root->addWidget(m_glowLine);

    /* ════════════ BODY ════════════ */
    QWidget *body = new QWidget;
    body->setObjectName("body");
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(20, 16, 20, 16);
    bodyLayout->setSpacing(14);

    /* ── Row 1: Language selector ── */
    {
        QHBoxLayout *row = new QHBoxLayout;
        row->setSpacing(10);

        QLabel *langLbl = new QLabel("🌐  INPUT LANGUAGE:");
        langLbl->setObjectName("rowLabel");
        row->addWidget(langLbl);

        m_langBox = new QComboBox;
        m_langBox->setObjectName("langBox");
        for (const LangInfo &li : m_langs)
            m_langBox->addItem(li.flag + "  " + li.name);
        m_langBox->setFixedWidth(260);
        row->addWidget(m_langBox);

        m_langTip = new QLabel(m_langs[0].tip);
        m_langTip->setObjectName("langTip");
        row->addWidget(m_langTip);

        row->addStretch();

        m_charCount = new QLabel("0 chars");
        m_charCount->setObjectName("charCount");
        row->addWidget(m_charCount);

        bodyLayout->addLayout(row);
    }

    /* ── Row 2: Editor area ── */
    {
        QHBoxLayout *editorRow = new QHBoxLayout;
        editorRow->setSpacing(12);

        /* INPUT */
        QVBoxLayout *inCol = new QVBoxLayout;
        inCol->setSpacing(6);
        QLabel *inLbl = new QLabel("INPUT");
        inLbl->setObjectName("edgeLabel");
        inCol->addWidget(inLbl);
        m_input = new QTextEdit;
        m_input->setObjectName("inputBox");
        m_input->setPlaceholderText("Type or paste your text here…");
        m_input->setMinimumHeight(175);
        inCol->addWidget(m_input);
        editorRow->addLayout(inCol, 1);

        /* CENTRE COLUMN */
        QVBoxLayout *midCol = new QVBoxLayout;
        midCol->setAlignment(Qt::AlignVCenter);
        midCol->setSpacing(8);

        m_btnTranslate = new QPushButton("TRANSLATE");
        m_btnTranslate->setObjectName("btnPrimary");
        m_btnTranslate->setFixedWidth(110);

        m_btnSwap = new QPushButton("⇄ SWAP");
        m_btnSwap->setObjectName("btnSecondary");
        m_btnSwap->setFixedWidth(110);

        m_btnClear = new QPushButton("✕ CLEAR");
        m_btnClear->setObjectName("btnDanger");
        m_btnClear->setFixedWidth(110);

        m_btnCopy = new QPushButton("⎘ COPY");
        m_btnCopy->setObjectName("btnInfo");
        m_btnCopy->setFixedWidth(110);

        midCol->addStretch();
        midCol->addWidget(m_btnTranslate);
        midCol->addWidget(m_btnSwap);
        midCol->addWidget(m_btnClear);
        midCol->addWidget(m_btnCopy);
        midCol->addStretch();
        editorRow->addLayout(midCol);

        /* OUTPUT */
        QVBoxLayout *outCol = new QVBoxLayout;
        outCol->setSpacing(6);
        QLabel *outLbl = new QLabel("OUTPUT");
        outLbl->setObjectName("edgeLabel");
        outCol->addWidget(outLbl);
        m_output = new QTextEdit;
        m_output->setObjectName("outputBox");
        m_output->setReadOnly(true);
        m_output->setPlaceholderText("Translation appears here…");
        m_output->setMinimumHeight(175);
        outCol->addWidget(m_output);
        editorRow->addLayout(outCol, 1);

        bodyLayout->addLayout(editorRow);
    }

    /* ── Row 3: Status bar ── */
    {
        QHBoxLayout *srow = new QHBoxLayout;
        srow->setSpacing(8);

        QLabel *dot = new QLabel("●");
        dot->setObjectName("statusDot");
        srow->addWidget(dot);

        m_status = new QLabel("Ready — select language and type your message.");
        m_status->setObjectName("statusText");
        srow->addWidget(m_status, 1);
        bodyLayout->addLayout(srow);
    }

    /* ── Row 4: Animation strip ── */
    {
        QHBoxLayout *hdr = new QHBoxLayout;
        hdr->setSpacing(12);

        QLabel *animLbl = new QLabel("⚡  LIVE DOT-DASH ANIMATION");
        animLbl->setObjectName("rowLabel");
        hdr->addWidget(animLbl);

        hdr->addStretch();

        m_btnPlay = new QPushButton("▶  PLAY");
        m_btnPlay->setObjectName("btnPlay");

        m_btnStop = new QPushButton("■  STOP");
        m_btnStop->setObjectName("btnStop");
        m_btnStop->setEnabled(false);

        hdr->addWidget(m_btnPlay);
        hdr->addWidget(m_btnStop);

        QLabel *spLbl = new QLabel("  SPEED:");
        spLbl->setObjectName("rowLabel");
        hdr->addWidget(spLbl);

        m_slider = new QSlider(Qt::Horizontal);
        m_slider->setRange(1, 5);
        m_slider->setValue(3);
        m_slider->setTickPosition(QSlider::TicksBelow);
        m_slider->setTickInterval(1);
        m_slider->setFixedWidth(120);
        hdr->addWidget(m_slider);

        m_lblSpeedVal = new QLabel(speedName());
        m_lblSpeedVal->setObjectName("speedVal");
        m_lblSpeedVal->setMinimumWidth(70);
        hdr->addWidget(m_lblSpeedVal);

        bodyLayout->addLayout(hdr);

        /* strip itself */
        m_strip = new MorseStrip;
        m_scroll = new QScrollArea;
        m_scroll->setObjectName("stripScroll");
        m_scroll->setWidget(m_strip);
        m_scroll->setWidgetResizable(true);
        m_scroll->setFixedHeight(48);
        m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        bodyLayout->addWidget(m_scroll);
    }

    /* ── Row 5: Language info cards ── */
    {
        QLabel *infoLbl = new QLabel("🌍  SUPPORTED LANGUAGES");
        infoLbl->setObjectName("rowLabel");
        bodyLayout->addWidget(infoLbl);

        QScrollArea *cardScroll = new QScrollArea;
        cardScroll->setObjectName("cardScroll");
        cardScroll->setFixedHeight(108);
        cardScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        cardScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        QWidget *cardRow = new QWidget;
        cardRow->setObjectName("cardRow");
        QHBoxLayout *cl = new QHBoxLayout(cardRow);
        cl->setContentsMargins(6, 6, 6, 6);
        cl->setSpacing(10);

        for (const LangInfo &li : m_langs) {
            QFrame *card = new QFrame;
            card->setObjectName("langCard");
            card->setFixedWidth(138);
            QVBoxLayout *cvl = new QVBoxLayout(card);
            cvl->setContentsMargins(10, 8, 10, 8);
            cvl->setSpacing(4);

            QLabel *flag = new QLabel(li.flag + "  " + li.name.split(' ')[0]);
            flag->setObjectName("cardFlag");
            flag->setWordWrap(false);

            QLabel *script = new QLabel(li.script);
            script->setObjectName("cardScript");

            QLabel *tip = new QLabel(li.tip);
            tip->setObjectName("cardTip");
            tip->setWordWrap(true);

            cvl->addWidget(flag);
            cvl->addWidget(script);
            cvl->addWidget(tip);
            cl->addWidget(card);
        }
        cl->addStretch();

        cardScroll->setWidget(cardRow);
        bodyLayout->addWidget(cardScroll);
    }

    root->addWidget(body, 1);

    /* ── Connections ── */
    connect(m_btnTranslate, &QPushButton::clicked, this, &MainWindow::onTranslate);
    connect(m_btnSwap,      &QPushButton::clicked, this, &MainWindow::onSwap);
    connect(m_btnPlay,      &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(m_btnStop,      &QPushButton::clicked, this, &MainWindow::onStop);
    connect(m_btnClear,     &QPushButton::clicked, this, &MainWindow::onClear);
    connect(m_btnCopy,      &QPushButton::clicked, this, &MainWindow::onCopy);
    connect(m_langBox,      QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onLangChanged);
    connect(m_slider,       &QSlider::valueChanged, this, &MainWindow::onSpeedChanged);
    connect(m_input,        &QTextEdit::textChanged, this, &MainWindow::onInputChanged);
}

/* ── Theme ── */
void MainWindow::applyTheme() {
    setStyleSheet(R"(

/* ━━━━━━━━━━━━━━━━━━━━━━━━ ROOT ━━━━━━━━━━━━━━━━━━━━━━━━ */
QMainWindow, QWidget {
    background: #070b14;
    color: #c9d1d9;
    font-family: "Courier New", Courier, monospace;
    font-size: 13px;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ HEADER ━━━━━━━━━━━━━━━━━━━━━━━━ */
QWidget#header {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #0d1627, stop:0.5 #0a1220, stop:1 #0d1627);
    border-bottom: 1px solid #1e3a5f;
}

QLabel#appName {
    font-family: "Courier New", monospace;
    font-size: 21px;
    font-weight: bold;
    color: #00e5ff;
    letter-spacing: 3px;
}
QLabel#appSub {
    font-size: 11px;
    color: #4a6fa5;
    letter-spacing: 1.5px;
}

QLabel#modePill {
    background: #00e5ff22;
    border: 1px solid #00e5ff66;
    border-radius: 12px;
    padding: 5px 16px;
    font-size: 12px;
    font-weight: bold;
    color: #00e5ff;
    letter-spacing: 2px;
}

QLabel#dsaBadge {
    font-size: 10px;
    color: #4a6fa5;
    letter-spacing: 1px;
}

QFrame#accentLine {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 transparent, stop:0.3 #00e5ff,
                stop:0.7 #00e5ff, stop:1 transparent);
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ BODY ━━━━━━━━━━━━━━━━━━━━━━━━ */
QWidget#body { background: #070b14; }

QLabel#rowLabel {
    font-size: 11px;
    font-weight: bold;
    color: #4a6fa5;
    letter-spacing: 2px;
}

QLabel#edgeLabel {
    font-size: 10px;
    font-weight: bold;
    color: #2a4a6f;
    letter-spacing: 3px;
}

QLabel#charCount {
    font-size: 11px;
    color: #4a6fa5;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ TEXT AREAS ━━━━━━━━━━━━━━━━━━━━━━━━ */
QTextEdit#inputBox {
    background: #0d1627;
    border: 1px solid #1e3a5f;
    border-radius: 10px;
    padding: 12px;
    color: #e6edf3;
    font-family: "Courier New", monospace;
    font-size: 14px;
    selection-background-color: #00e5ff44;
    selection-color: #e6edf3;
}
QTextEdit#inputBox:focus {
    border: 1px solid #00e5ff88;
}

QTextEdit#outputBox {
    background: #071a0f;
    border: 1px solid #0a3020;
    border-radius: 10px;
    padding: 12px;
    color: #00ff9f;
    font-family: "Courier New", monospace;
    font-size: 15px;
    letter-spacing: 2px;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ BUTTONS ━━━━━━━━━━━━━━━━━━━━━━━━ */
QPushButton {
    border-radius: 7px;
    padding: 9px 14px;
    font-family: "Courier New", monospace;
    font-weight: bold;
    font-size: 12px;
    border: 1px solid #1e3a5f;
    background: #0d1627;
    color: #c9d1d9;
    letter-spacing: 1px;
}
QPushButton:hover  { background: #1a2a4a; }
QPushButton:pressed{ background: #0a1525; }
QPushButton:disabled { color: #2a3a5f; border-color: #0d1627; }

QPushButton#btnPrimary {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #0a3a5a, stop:1 #062540);
    border: 1px solid #00e5ff88;
    color: #00e5ff;
}
QPushButton#btnPrimary:hover {
    background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #0d4a70, stop:1 #083050);
    border-color: #00e5ff;
}

QPushButton#btnSecondary {
    border-color: #1e3a5f;
    color: #58a6ff;
}
QPushButton#btnSecondary:hover { border-color: #58a6ff; color: #79baff; }

QPushButton#btnDanger {
    border-color: #3a1a1a;
    color: #f85149;
}
QPushButton#btnDanger:hover { border-color: #f85149; }

QPushButton#btnInfo {
    border-color: #1a3a2a;
    color: #00ff9f;
}
QPushButton#btnInfo:hover { border-color: #00ff9f; }

QPushButton#btnPlay {
    background: #071a0f;
    border: 1px solid #00ff9f55;
    color: #00ff9f;
}
QPushButton#btnPlay:hover { border-color: #00ff9f; }

QPushButton#btnStop {
    background: #1a0707;
    border: 1px solid #f8514955;
    color: #f85149;
}
QPushButton#btnStop:hover { border-color: #f85149; }
QPushButton#btnStop:disabled { color: #3a2020; border-color: #1a0707; }

/* ━━━━━━━━━━━━━━━━━━━━━━━━ COMBO / SLIDER ━━━━━━━━━━━━━━━━━━━━━━━━ */
QComboBox#langBox {
    background: #0d1627;
    border: 1px solid #1e3a5f;
    border-radius: 7px;
    padding: 6px 12px;
    color: #c9d1d9;
    font-family: "Courier New", monospace;
}
QComboBox#langBox:focus { border-color: #00e5ff66; }
QComboBox#langBox QAbstractItemView {
    background: #0d1627;
    border: 1px solid #1e3a5f;
    selection-background-color: #00e5ff22;
    color: #c9d1d9;
}
QComboBox#langBox::drop-down { border: none; }

QSlider::groove:horizontal {
    height: 4px; background: #1e3a5f; border-radius: 2px;
}
QSlider::handle:horizontal {
    width: 14px; height: 14px; margin: -5px 0;
    background: #00e5ff; border-radius: 7px;
}
QSlider::sub-page:horizontal {
    background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                stop:0 #0050a0, stop:1 #00e5ff);
    border-radius: 2px;
}

QLabel#speedVal {
    color: #00e5ff;
    font-weight: bold;
    font-size: 11px;
}

QLabel#langTip {
    color: #4a6fa5;
    font-size: 11px;
    font-style: italic;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ STATUS ━━━━━━━━━━━━━━━━━━━━━━━━ */
QLabel#statusDot { color: #00ff9f; font-size: 8px; }
QLabel#statusText { color: #4a6fa5; font-size: 12px; }

/* ━━━━━━━━━━━━━━━━━━━━━━━━ ANIMATION STRIP ━━━━━━━━━━━━━━━━━━━━━━━━ */
QScrollArea#stripScroll {
    background: #0a1220;
    border: 1px solid #1e3a5f;
    border-radius: 8px;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ LANGUAGE CARDS ━━━━━━━━━━━━━━━━━━━━━━━━ */
QScrollArea#cardScroll {
    background: transparent;
    border: none;
}
QWidget#cardRow { background: transparent; }

QFrame#langCard {
    background: #0d1627;
    border: 1px solid #1e3a5f;
    border-radius: 10px;
}
QFrame#langCard:hover {
    border-color: #00e5ff55;
    background: #101e35;
}

QLabel#cardFlag {
    font-size: 13px;
    font-weight: bold;
    color: #c9d1d9;
}
QLabel#cardScript {
    font-size: 10px;
    color: #00e5ff;
    letter-spacing: 1px;
    font-weight: bold;
}
QLabel#cardTip {
    font-size: 9px;
    color: #4a6fa5;
    line-height: 1.4;
    font-family: "Segoe UI", Arial, sans-serif;
}

/* ━━━━━━━━━━━━━━━━━━━━━━━━ SCROLLBARS ━━━━━━━━━━━━━━━━━━━━━━━━ */
QScrollBar:horizontal {
    height: 6px; background: #0d1627;
}
QScrollBar::handle:horizontal {
    background: #1e3a5f; border-radius: 3px; min-width: 20px;
}
QScrollBar::handle:horizontal:hover { background: #00e5ff55; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0; }

QScrollBar:vertical {
    width: 6px; background: #0d1627;
}
QScrollBar::handle:vertical {
    background: #1e3a5f; border-radius: 3px; min-height: 20px;
}
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height:0; }

    )");
}

/* ═══════════════════════════════════════════════════════════════════
   SLOTS
   ═══════════════════════════════════════════════════════════════════ */

void MainWindow::setStatus(const QString &msg, const QString &color) {
    m_status->setText(msg);
    m_status->setStyleSheet(QString("font-size:12px;color:%1;").arg(color));
}

void MainWindow::updateMode() {
    m_modeLabel->setText(m_toMorse ? "TEXT  →  MORSE" : "MORSE  →  TEXT");
    m_input->setPlaceholderText(m_toMorse
        ? "Type or paste your text here…"
        : "Paste morse code here…  e.g.  ·− −··· −·−·");
}

int  MainWindow::speedMs()   const {
    static int ms[] = {600,380,220,120,55};
    return ms[m_slider->value()-1];
}
QString MainWindow::speedName() const {
    static const char *n[] = {"Very Slow","Slow","Normal","Fast","Blazing"};
    return QString(n[m_slider->value()-1]);
}

void MainWindow::onInputChanged() {
    int n = m_input->toPlainText().length();
    m_charCount->setText(QString("%1 chars").arg(n));
}

void MainWindow::onLangChanged(int idx) {
    if (idx >= 0 && idx < m_langs.size())
        m_langTip->setText(m_langs[idx].tip);
}

/* Translate */
void MainWindow::onTranslate() {
    QString raw = m_input->toPlainText().trimmed();
    if (raw.isEmpty()) { setStatus("⚠  Input is empty.", "#f85149"); return; }

    int langIdx = m_langBox->currentIndex();

    if (m_toMorse) {
        /* Apply transliteration if needed */
        QString processable = raw;
        if (langIdx == 1)  processable = m_tree->transliterate(raw, m_tree->hindiMap);
        else if (langIdx == 2)  processable = m_tree->transliterate(raw, m_tree->frenchMap);
        else if (langIdx == 3)  processable = m_tree->transliterate(raw, m_tree->spanishMap);
        else if (langIdx == 4)  processable = m_tree->transliterate(raw, m_tree->germanMap);
        else if (langIdx == 5)  processable = m_tree->transliterate(raw, m_tree->portugueseMap);
        else if (langIdx == 6)  processable = m_tree->transliterate(raw, m_tree->russianMap);
        else if (langIdx == 7)  processable = m_tree->transliterate(raw, m_tree->greekMap);
        else if (langIdx == 8)  processable = m_tree->transliterate(raw, m_tree->arabicMap);
        else if (langIdx == 9)  processable = m_tree->transliterate(raw, m_tree->japaneseMap);
        else if (langIdx == 10) processable = m_tree->transliterate(raw, m_tree->chineseMap);

        m_currentMorse = m_tree->encodeText(processable);
        if (m_currentMorse.isEmpty()) {
            setStatus("⚠  No translatable characters found.", "#f85149"); return;
        }
        m_output->setPlainText(m_currentMorse);
    } else {
        QString decoded = m_tree->decodeText(raw);
        m_output->setPlainText(decoded);
        m_currentMorse = raw;
    }

    m_strip->loadMorse(m_currentMorse);
    m_strip->resetAll();
    setStatus(QString("✓  Translated %1 character(s) using Binary Search Tree.").arg(raw.length()));
}

void MainWindow::onSwap() {
    m_toMorse = !m_toMorse;
    updateMode();
    QString tmp = m_input->toPlainText();
    m_input->setPlainText(m_output->toPlainText());
    m_output->setPlainText(tmp);
    m_currentMorse.clear();
    m_strip->loadMorse("");
    setStatus("Mode swapped.");
}

void MainWindow::onPlay() {
    if (m_currentMorse.isEmpty()) {
        setStatus("⚠  Translate something first.", "#f85149"); return;
    }
    m_strip->resetAll();
    m_playIdx = 0;
    m_timer->start(speedMs());
    m_btnPlay->setEnabled(false);
    m_btnStop->setEnabled(true);
    setStatus("▶  Playing animation…");
}

void MainWindow::onStop() {
    m_timer->stop();
    m_strip->resetAll();
    m_btnPlay->setEnabled(true);
    m_btnStop->setEnabled(false);
    setStatus("■  Stopped.");
}

void MainWindow::onTick() {
    if (m_playIdx >= m_strip->count()) {
        m_timer->stop();
        m_btnPlay->setEnabled(true);
        m_btnStop->setEnabled(false);
        setStatus("✓  Animation complete.");
        return;
    }
    m_strip->flashAt(m_playIdx++);
}

void MainWindow::onSpeedChanged(int) {
    m_lblSpeedVal->setText(speedName());
    if (m_timer->isActive()) m_timer->setInterval(speedMs());
}

void MainWindow::onClear() {
    m_timer->stop();
    m_input->clear();
    m_output->clear();
    m_strip->loadMorse("");
    m_currentMorse.clear();
    m_btnPlay->setEnabled(true);
    m_btnStop->setEnabled(false);
    setStatus("Cleared.");
}

void MainWindow::onCopy() {
    QString t = m_output->toPlainText();
    if (t.isEmpty()) { setStatus("⚠  Nothing to copy.", "#f85149"); return; }
    QApplication::clipboard()->setText(t);
    setStatus("⎘  Copied to clipboard!", "#58a6ff");
}
