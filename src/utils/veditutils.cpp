#include "veditutils.h"

#include <QTextDocument>
#include <QDebug>

#include "vutils.h"

void VEditUtils::removeBlock(QTextBlock &p_block, QString *p_text)
{
    QTextCursor cursor(p_block);
    removeBlock(cursor, p_text);
}

void VEditUtils::removeBlock(QTextCursor &p_cursor, QString *p_text)
{
    const QTextDocument *doc = p_cursor.document();
    int blockCount = doc->blockCount();
    int blockNum = p_cursor.block().blockNumber();

    p_cursor.select(QTextCursor::BlockUnderCursor);
    if (p_text) {
        *p_text = p_cursor.selectedText() + "\n";
    }

    p_cursor.deleteChar();

    // Deleting the first block will leave an empty block.
    // Deleting the last empty block will not work with deleteChar().
    if (blockCount == doc->blockCount()) {
        if (blockNum == blockCount - 1) {
            // The last block.
            p_cursor.deletePreviousChar();
        } else {
            p_cursor.deleteChar();
        }
    }

    if (p_cursor.block().blockNumber() < blockNum) {
        p_cursor.movePosition(QTextCursor::NextBlock);
    }

    p_cursor.movePosition(QTextCursor::StartOfBlock);
}

bool VEditUtils::insertBlockWithIndent(QTextCursor &p_cursor)
{
    V_ASSERT(!p_cursor.hasSelection());
    p_cursor.insertBlock();
    return indentBlockAsPreviousBlock(p_cursor);
}

bool VEditUtils::insertListMarkAsPreviousBlock(QTextCursor &p_cursor)
{
    bool ret = false;
    QTextBlock block = p_cursor.block();
    QTextBlock preBlock = block.previous();
    if (!preBlock.isValid()) {
        return false;
    }

    QString text = preBlock.text();
    QRegExp regExp("^\\s*(-|\\d+\\.)\\s");
    int regIdx = regExp.indexIn(text);
    if (regIdx != -1) {
        ret = true;
        V_ASSERT(regExp.captureCount() == 1);
        QString markText = regExp.capturedTexts()[1];
        if (markText == "-") {
            // Insert - in front.
            p_cursor.insertText("- ");
        } else {
            // markText is like "123.".
            V_ASSERT(markText.endsWith('.'));
            bool ok = false;
            int num = markText.left(markText.size() - 1).toInt(&ok, 10);
            V_ASSERT(ok);
            num++;
            p_cursor.insertText(QString::number(num, 10) + ". ");
        }
    }

    return ret;

}

bool VEditUtils::indentBlockAsPreviousBlock(QTextCursor &p_cursor)
{
    bool changed = false;
    QTextBlock block = p_cursor.block();
    if (block.blockNumber() == 0) {
        // The first block.
        return false;
    }

    QTextBlock preBlock = block.previous();
    QString text = preBlock.text();
    QRegExp regExp("(^\\s*)");
    regExp.indexIn(text);
    V_ASSERT(regExp.captureCount() == 1);
    QString leadingSpaces = regExp.capturedTexts()[1];

    moveCursorFirstNonSpaceCharacter(p_cursor, QTextCursor::MoveAnchor);
    if (!p_cursor.atBlockStart()) {
        p_cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        p_cursor.removeSelectedText();
        changed = true;
    }

    if (!leadingSpaces.isEmpty()) {
        p_cursor.insertText(leadingSpaces);
        changed = true;
    }

    p_cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);

    return changed;
}

void VEditUtils::moveCursorFirstNonSpaceCharacter(QTextCursor &p_cursor,
                                                  QTextCursor::MoveMode p_mode)
{
    QTextBlock block = p_cursor.block();
    QString text = block.text();
    int idx = 0;
    for (; idx < text.size(); ++idx) {
        if (text[idx].isSpace()) {
            continue;
        } else {
            break;
        }
    }

    p_cursor.setPosition(block.position() + idx, p_mode);
}

