#include <QtTest>

#include "calculator.h"

#include <cmath>
#include <limits>

namespace {
// Drive the calculator with a space-separated key script, e.g. "3 enter 4 +".
void press(Calculator &c, const QString &script) {
    const QStringList keys = script.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString &k : keys) {
        if (k == "enter") c.enter();
        else if (k == "+") c.add();
        else if (k == "-") c.subtract();
        else if (k == "*") c.multiply();
        else if (k == "/") c.divide();
        else if (k == "swap") c.swap();
        else if (k == "roll") c.rollDown();
        else if (k == "rollup") c.rollUp();
        else if (k == "drop") c.drop();
        else if (k == "bs") c.backspace();
        else if (k == "neg") c.negate();
        else if (k == "inv") c.reciprocal();
        else if (k == "sqrt") c.sqrt();
        else if (k == "c") c.clearKey();
        else if (k == "ac") c.clearAll();
        else if (k == "mr") c.memoryRecall();
        else if (k == "m+") c.memoryAdd(1);
        else if (k == "m-") c.memoryAdd(-1);
        else for (const QChar ch : k) c.digit(QString(ch));
    }
}

QVector<double> stack(std::initializer_list<double> values) { return QVector<double>(values); }
}

class CalculatorTest : public QObject {
    Q_OBJECT

private slots:
    void arithmeticCommitsEntry() {
        Calculator c;
        press(c, "3 enter 4 +");
        QCOMPARE(c.stack(), stack({7}));
        press(c, "2 -");
        QCOMPARE(c.stack(), stack({5}));
    }

    void enterDuplicatesX() {
        Calculator c;
        press(c, "5 enter enter *");
        QCOMPARE(c.stack(), stack({25}));
    }

    void swapAndRoll() {
        Calculator c;
        press(c, "25 enter 1 enter 2 swap");
        QCOMPARE(c.stack(), stack({25, 2, 1}));
        press(c, "roll");
        QCOMPARE(c.stack(), stack({1, 25, 2}));
        press(c, "rollup");
        QCOMPARE(c.stack(), stack({25, 2, 1}));
    }

    void dropClearsEntryBeforePopping() {
        Calculator c;
        press(c, "25 enter 2 enter 1 enter 9 drop");
        QCOMPARE(c.stack(), stack({25, 2, 1}));
        QCOMPARE(c.entry(), QString());
        press(c, "drop");
        QCOMPARE(c.stack(), stack({25, 2}));
    }

    void refusedOperationsLeaveStackIntact() {
        Calculator c;
        press(c, "25 enter 2 enter 0 /");
        QCOMPARE(c.message(), QStringLiteral("Error"));
        QCOMPARE(c.stack(), stack({25, 2, 0}));

        Calculator d;
        press(d, "+");
        QCOMPARE(d.message(), QStringLiteral("Need two values"));
        QCOMPARE(d.stack(), stack({}));

        Calculator e;
        press(e, "1 neg sqrt");
        QCOMPARE(e.message(), QStringLiteral("Error"));
        QCOMPARE(e.stack(), stack({-1}));
    }

    void entryEditing() {
        Calculator c;
        press(c, "0 5");
        QCOMPARE(c.entry(), QStringLiteral("5"));
        press(c, ". . 5");
        QCOMPARE(c.entry(), QStringLiteral("5.5"));
        press(c, "neg");
        QCOMPARE(c.entry(), QStringLiteral("-5.5"));
        press(c, "neg");
        QCOMPARE(c.entry(), QStringLiteral("5.5"));
        press(c, "e 3 neg");
        QCOMPARE(c.entry(), QStringLiteral("5.5e-3"));
        press(c, "enter");
        QCOMPARE(c.stack(), stack({0.0055}));
        press(c, "bs");
        QCOMPARE(c.stack(), stack({}));

        Calculator d;
        press(d, ".");
        QCOMPARE(d.entry(), QStringLiteral("0."));
        press(d, "bs bs");
        QCOMPARE(d.entry(), QString());
    }

    void negateOnStack() {
        Calculator c;
        press(c, "4 enter neg");
        QCOMPARE(c.stack(), stack({-4}));
    }

    void unaryOperations() {
        Calculator c;
        press(c, "16 sqrt");
        QCOMPARE(c.stack(), stack({4}));
        press(c, "inv");
        QCOMPARE(c.stack(), stack({0.25}));
    }

    void clearKeyIsTwoStage() {
        Calculator c;
        press(c, "5 enter 9 c");
        QCOMPARE(c.stack(), stack({5}));
        QCOMPARE(c.entry(), QString());
        QVERIFY(c.clearArmed());

        // Any other key disarms it.
        press(c, "7");
        QVERIFY(!c.clearArmed());
        press(c, "enter c");
        QCOMPARE(c.stack(), stack({5, 0}));
        QVERIFY(c.liftDisabled());
        press(c, "c");
        QCOMPARE(c.stack(), stack({}));
        QVERIFY(!c.clearArmed());
    }

    void zeroedXIsReplacedNotLifted() {
        Calculator c;
        press(c, "5 enter c 7 enter");
        QCOMPARE(c.stack(), stack({7}));

        press(c, "c");
        c.pushNumber(3);
        QCOMPARE(c.stack(), stack({3}));

        press(c, "c enter");
        QCOMPARE(c.stack(), stack({0, 0}));  // enter keeps the 0 (dup)

        Calculator d;
        press(d, "c");
        QCOMPARE(d.stack(), stack({}));
    }

    void memoryAccumulates() {
        Calculator c;
        press(c, "10 m+ 3 m- mr");
        QCOMPARE(c.memory(), 7.0);
        QCOMPARE(c.stack(), stack({10, 3, 7}));
        QVERIFY(c.memoryHeld());

        press(c, "c c");
        QCOMPARE(c.memory(), 0.0);
        QVERIFY(!c.memoryHeld());
        QCOMPARE(c.stack(), stack({}));
        press(c, "mr");
        QCOMPARE(c.stack(), stack({0}));
    }

    void memoryRecallAfterClearReplacesZero() {
        Calculator c;
        press(c, "4 m+ 9 enter c mr");
        QCOMPARE(c.stack(), stack({4, 4}));
    }

    void pasteParsesNumbers() {
        Calculator c;
        QVERIFY(c.pasteText(QStringLiteral(" 1,234.5\n")));
        QCOMPARE(c.stack(), stack({1234.5}));
        QVERIFY(!c.pasteText(QStringLiteral("abc")));
        QCOMPARE(c.message(), QStringLiteral("Not a number"));
        // Pasting commits a pending entry first.
        press(c, "2");
        QVERIFY(c.pasteText(QStringLiteral("9")));
        QCOMPARE(c.stack(), stack({1234.5, 2, 9}));
    }

    void formatsNumbers_data() {
        QTest::addColumn<double>("value");
        QTest::addColumn<QString>("text");
        QTest::newRow("fp noise")        << 0.1 + 0.2 << "0.3";
        QTest::newRow("12 sig")          << 43.0716627634 << "43.0716627634";
        QTest::newRow("negative")        << -43.0716627634 << "-43.0716627634";
        QTest::newRow("third")           << 1.0 / 3 << "0.333333333333";
        QTest::newRow("neg third")       << -1.0 / 3 << "-0.333333333333";
        QTest::newRow("12 digit int")    << 123456789012.0 << "123456789012";
        QTest::newRow("13 digit int")    << 1234567890123.0 << "1234567890123";
        QTest::newRow("15 digit int")    << 999999999999999.0 << "999999999999999";
        QTest::newRow("neg 15 digit")    << -999999999999999.0 << "-999999999999999";
        QTest::newRow("1e15")            << 1e15 << "1e+15";
        QTest::newRow("16 digit int")    << 1234567890123456.0 << "1.2345678901e+15";
        QTest::newRow("1e20")            << 1e20 << "1e+20";
        QTest::newRow("wide positive")   << 123456789012345680000.0 << "1.2345678901e+20";
        QTest::newRow("wide negative")   << -123456789012345680000.0 << "-1.23456789e+20";
        QTest::newRow("1e21")            << 1e21 << "1e+21";
        QTest::newRow("small exp")       << 1.5e-7 << "1.5e-7";
        QTest::newRow("small fixed")     << 0.000001234 << "0.000001234";
        QTest::newRow("neg zero")        << -0.0 << "0";
        QTest::newRow("huge")            << 1e300 << "1e+300";
        QTest::newRow("2^64")            << std::pow(2.0, 64) << "1.8446744074e+19";
        QTest::newRow("tiny")            << 1 / 7e10 << "1.4285714286e-11";
        QTest::newRow("inf")             << std::numeric_limits<double>::infinity() << "Error";
    }

    void formatsNumbers() {
        QFETCH(double, value);
        QFETCH(QString, text);
        QCOMPARE(Calculator::formatNumber(value), text);
        QVERIFY(text.size() <= 16);
    }

    void levelsShowStackAndEntry() {
        Calculator c;
        press(c, "12 enter 3");
        const QVariantList rows = c.levels();
        QCOMPARE(rows.size(), 4);
        QCOMPARE(rows.at(0).toMap().value("level").toInt(), 4);
        QCOMPARE(rows.at(2).toMap().value("value").toString(), QStringLiteral("12"));
        QCOMPARE(rows.at(3).toMap().value("value").toString(), QStringLiteral("3"));
        QVERIFY(rows.at(3).toMap().value("typing").toBool());
        QVERIFY(!rows.at(2).toMap().value("typing").toBool());
    }
};

QTEST_MAIN(CalculatorTest)
#include "tst_calculator.moc"
