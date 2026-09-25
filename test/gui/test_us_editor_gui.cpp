#include <QAction>
#include <QTextEdit>

#include "us3_gui_test_main.h"
#include "us_editor.h"

class US_EditorGuiTest : public QObject
{
    Q_OBJECT

private slots:
    void acceptsKeyboardInputAndClearsDocument()
    {
        US_Editor editor(US_Editor::DEFAULT);
        editor.show();

        QVERIFY(editor.isVisible());
        QVERIFY(editor.e != nullptr);

        editor.e->setFocus();
        QTest::keyClicks(editor.e, "headless QTest");
        QCOMPARE(editor.e->toPlainText(), QString("headless QTest"));

        QAction* clearAction = nullptr;
        for (QAction* action : editor.findChildren<QAction*>())
        {
            QString text = action->text();
            text.remove('&');
            if (text == "Clear")
            {
                clearAction = action;
                break;
            }
        }

        QVERIFY2(clearAction != nullptr, "The editor Clear action was not found");
        clearAction->trigger();
        QCOMPARE(editor.e->toPlainText(), QString());
    }
};

US3_GUI_TEST_MAIN(US_EditorGuiTest)

#include "test_us_editor_gui.moc"
