/*
 * Copyright 2013  Sebastian Gottfried <sebastiangottfried@web.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "customlessoneditordialog.h"

#include <QPushButton>

#include "core/keyboardlayout.h"
#include "core/key.h"
#include "core/keychar.h"
#include "core/lesson.h"
#include "editor/lessontexthighlighter.h"

CustomLessonEditorDialog::CustomLessonEditorDialog(QWidget* parent) :
    QDialog(parent),
    Ui::CustomLessonEditorDialog(),
    m_lesson(0),
    m_keyboardLayout(0)
{
    this->setupUi(this);

    connect(m_titleLineEdit, &QLineEdit::textChanged, this, &CustomLessonEditorDialog::onTitleChanged);
    connect(m_lessonTextEditor->textEdit(), &QTextEdit::textChanged, this, &CustomLessonEditorDialog::onTextChanged);
    connect(this, &CustomLessonEditorDialog::validChanged, m_buttonBox->button(QDialogButtonBox::Ok), &QPushButton::setEnabled);
}

Lesson* CustomLessonEditorDialog::lesson() const
{
    return m_lesson;
}

void CustomLessonEditorDialog::setLesson(Lesson* lesson)
{
    if (lesson != m_lesson)
    {
        if (m_lesson)
        {
            m_lesson->disconnect(this);
        }

        m_lesson = lesson;

        if (lesson)
        {
            updateTitle();
            updateText();

            connect(lesson, &Lesson::titleChanged, this, &CustomLessonEditorDialog::updateTitle);
            connect(lesson, &Lesson::textChanged, this, &CustomLessonEditorDialog::updateText);
        }
    }
}

KeyboardLayout* CustomLessonEditorDialog::keyboardLayout() const
{
    return m_keyboardLayout;
}

void CustomLessonEditorDialog::setKeyboardLayout(KeyboardLayout* keyboardLayout)
{
    if (keyboardLayout != m_keyboardLayout)
    {
        if (m_keyboardLayout)
        {
            m_keyboardLayout->disconnect(this);
        }

        m_keyboardLayout = keyboardLayout;
        updateAllowedCharacters();

        if (m_keyboardLayout)
        {
            connect(m_keyboardLayout, &Resource::isValidChanged, this, &CustomLessonEditorDialog::updateAllowedCharacters);
        }
    }
}

void CustomLessonEditorDialog::updateTitle()
{
    const QString title = m_lesson->title();

    if (title != m_titleLineEdit->text())
    {
        m_titleLineEdit->setText(title);
    }
}

void CustomLessonEditorDialog::updateText()
{
    const QString text = m_lesson->text();

    if (text != m_lessonTextEditor->textEdit()->toPlainText())
    {
        m_lessonTextEditor->textEdit()->setPlainText(text);
    }
}

void CustomLessonEditorDialog::updateAllowedCharacters()
{
    if (!m_keyboardLayout || !m_keyboardLayout->isValid())
    {
        m_lessonTextEditor->highlighter()->setAllowedCharacters(QString());
        return;
    }

    QString chars;

    for (int i = 0; i < m_keyboardLayout->keyCount(); i++)
    {
        Key* const key = qobject_cast<Key*>(m_keyboardLayout->key(i));

        if (!key)
            continue;

        for (int j = 0; j < key->keyCharCount(); j++)
        {
            KeyChar* const keyChar = key->keyChar(j);

            chars += keyChar->value();
        }
    }

    m_lessonTextEditor->highlighter()->setAllowedCharacters(chars);
}

void CustomLessonEditorDialog::onTitleChanged()
{
    if (!m_lesson)
        return;

    m_lesson->setTitle(m_titleLineEdit->text());

    emitValidChanged();
}

void CustomLessonEditorDialog::onTextChanged()
{
    if (!m_lesson)
        return;

    m_lesson->setText(m_lessonTextEditor->textEdit()->toPlainText());

    emitValidChanged();
}

void CustomLessonEditorDialog::emitValidChanged()
{
    const QString title = m_lesson->title();
    const QString text = m_lesson->text();
    const bool valid = !title.isEmpty() && !text.isEmpty();

    emit validChanged(valid);
}
