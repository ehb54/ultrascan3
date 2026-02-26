#include <gtest/gtest.h>
#include "qt_test_base.h"
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

class US_RegexTest : public QtTestBase {
protected:
    struct PatternEntry {
        QString file;
        int line;
        QString pattern;
        QString context;
        bool valid;
        QString error;
    };

    QString fixPattern(QString pattern_str) {
        if (pattern_str.startsWith('R')) {
            // strip R("(...)") or R"(...)"
            // Usually it's R"(pattern)"
            int firstQuote = pattern_str.indexOf('"');
            int lastQuote = pattern_str.lastIndexOf('"');
            if (firstQuote != -1 && lastQuote > firstQuote) {
                // Check for R"(...) or R"xxx(...)xxx"
                int openParen = pattern_str.indexOf('(', firstQuote);
                int closeParen = pattern_str.lastIndexOf(')', lastQuote);
                if (openParen != -1 && closeParen > openParen) {
                    return pattern_str.mid(openParen + 1, closeParen - openParen - 1);
                }
            }
        } else {
            // Remove surrounding quotes
            if ((pattern_str.startsWith('"') && pattern_str.endsWith('"')) ||
                (pattern_str.startsWith('\'') && pattern_str.endsWith('\''))) {
                pattern_str = pattern_str.mid(1, pattern_str.length() - 2);
            }
            pattern_str.replace("\\\\", "\\");
            pattern_str.replace("\\\"", "\"");
        }
        return pattern_str;
    }

    QString findQStringDefinition(const QStringList& lines, int currentLine, const QString& varName) {
        int start = qMax(0, currentLine - 50);
        for (int i = currentLine - 2; i >= start; --i) {
            QString line = lines[i];
            
            // Pattern 1: QString var_name = "pattern"
            QRegularExpression rx1(QString("QString\\s+%1\\s*=\\s*(R?[\"'][^\"']+[\"'])").arg(varName));
            auto match1 = rx1.match(line);
            if (match1.hasMatch()) return match1.captured(1);

            // Pattern 2: QString var_name ("pattern")
            QRegularExpression rx2(QString("QString\\s+%1\\s*\\(\\s*(R?[\"'][^\"']+[\"'])\\s*\\)").arg(varName));
            auto match2 = rx2.match(line);
            if (match2.hasMatch()) return match2.captured(1);

            // Pattern 3: var_name = "pattern"
            QRegularExpression rx3(QString("%1\\s*=\\s*[\"']([^\"']+)[\"']").arg(varName));
            auto match3 = rx3.match(line);
            if (match3.hasMatch()) return "\"" + match3.captured(1) + "\"";
        }
        return QString("<variable: %1>").arg(varName);
    }

    QList<PatternEntry> extractPatterns(const QString& filePath) {
        QList<PatternEntry> patterns;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return patterns;

        QTextStream in(&file);
        QStringList lines;
        while (!in.atEnd()) {
            lines.append(in.readLine());
        }
        file.close();

        QRegularExpression rx1(R"(QRegularExpression\s*\w*\s*\(\s*(R?["'][^"']+["'])\s*[,\)])");
        QRegularExpression rx2(R"(QRegularExpression\s*\w*\s*\(\s*([a-zA-Z_]\w*)\s*[,\)])");
        QRegularExpression rx3(R"(\.setPattern\s*\(\s*(R?["'][^"']+["'])\s*[,\)])");
        QRegularExpression rx4(R"(\.setPattern\s*\(\s*([a-zA-Z_]\w*)\s*[,\)])");

        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            int lineNum = i + 1;

            auto match1 = rx1.match(line);
            if (match1.hasMatch()) {
                QString pattern = match1.captured(1);
                patterns.append({filePath, lineNum, pattern, line.trimmed(), true, ""});
                continue;
            }

            auto match2 = rx2.match(line);
            if (match2.hasMatch()) {
                QString varName = match2.captured(1);
                QString pattern = findQStringDefinition(lines, lineNum, varName);
                patterns.append({filePath, lineNum, pattern, line.trimmed(), true, ""});
                continue;
            }

            auto match3 = rx3.match(line);
            if (match3.hasMatch()) {
                QString pattern = match3.captured(1);
                patterns.append({filePath, lineNum, pattern, line.trimmed(), true, ""});
                continue;
            }

            auto match4 = rx4.match(line);
            if (match4.hasMatch()) {
                QString varName = match4.captured(1);
                QString pattern = findQStringDefinition(lines, lineNum, varName);
                patterns.append({filePath, lineNum, pattern, line.trimmed(), true, ""});
                continue;
            }
        }
        return patterns;
    }
};

TEST_F(US_RegexTest, ValidateAllRegexPatterns) {
    QStringList targetDirs = {"utils", "gui", "somo", "us_somo", "programs"};
    QList<PatternEntry> allPatterns;
    const QString rootPath (US3_SOURCE_DIR);
    for (const QString& dir : targetDirs) {
        QDirIterator it(rootPath + "/" + dir, {"*.cpp", "*.h", "*.hpp", "*.cc"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allPatterns.append(extractPatterns(it.next()));
        }
    }
    if (allPatterns.isEmpty()) {
        FAIL() << "No regex patterns found!";
    }

    int invalidCount = 0;
    for (auto&[file, line, pattern, context, valid, error] : allPatterns) {
        if (pattern.startsWith("<variable")) {
            continue;
        }

        QString fixedPattern = fixPattern(pattern);
        QRegularExpression re(fixedPattern);
        if (!re.isValid()) {
            invalidCount++;
            valid = false;
            error = re.errorString();
            
            ADD_FAILURE() << "Invalid regex pattern found!"
                          << "\n  File: " << file.toStdString()
                          << "\n  Line: " << line
                          << "\n  Raw Pattern: " << pattern.toStdString()
                          << "\n  Fixed Pattern: " << fixedPattern.toStdString()
                          << "\n  Error: " << error.toStdString()
                          << "\n  Context: " << context.toStdString();
        }
    }

    if (invalidCount > 0) {
        FAIL() << "Found " << invalidCount << " invalid regex patterns.";
    }

}
