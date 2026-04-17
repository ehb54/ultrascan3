#include <gtest/gtest.h>
#include "qt_test_base.h"
#include <QDirIterator>
#include <QFile>
#include <QRegularExpression>
#include <QTextStream>
#include <QFileInfo>
#include <iostream>

class US_HelpPagesTest : public QtTestBase {
protected:
    struct HelpCall {
        QString file;
        int line;
        QString helpString;
        QString context;
    };

    QList<HelpCall> extractHelpCalls(const QString& filePath) {
        QList<HelpCall> calls;
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return calls;

        QTextStream in(&file);
        int lineNum = 0;
        // Search for show_help("...") or show_help('...')
        // Case-insensitive as sometimes it's showHelp or showhelp
        QRegularExpression rx(R"(show_help\s*\(\s*["']([^"']+)["']\s*\))", QRegularExpression::CaseInsensitiveOption);

        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNum++;
            
            auto it = rx.globalMatch(line);
            while (it.hasNext()) {
                auto match = it.next();
                QString helpString = match.captured(1);
                calls.append({filePath, lineNum, helpString, line.trimmed()});
            }
        }
        file.close();
        return calls;
    }
};

TEST_F(US_HelpPagesTest, ValidateAllHelpPagesExist) {
    // Directories to scan for show_help() calls
    QStringList targetDirs = {"gui", "programs", "utils"};
    QList<HelpCall> allCalls;
    const QString rootPath(US3_SOURCE_DIR);
    
    for (const QString& dir : targetDirs) {
        QString dirPath = rootPath + "/" + dir;
        if (!QFile::exists(dirPath)) continue;
        
        QDirIterator it(dirPath, {"*.cpp", "*.h", "*.hpp", "*.cc"}, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
            allCalls.append(extractHelpCalls(it.next()));
        }
    }

    if (allCalls.isEmpty()) {
        std::cout << "No show_help() calls found in scanned directories." << std::endl;
        return;
    }

    const QString sourcePath = rootPath + "/doc/manual/source/";
    int missingCount = 0;
    QStringList missingFiles;

    for (const auto& call : allCalls) {
        QString relPath = call.helpString;
        
        // Remove "manual/" prefix if it exists
        if (relPath.startsWith("manual/")) {
            relPath = relPath.mid(7);
        }
        
        // Help string can include anchors like #anchor, remove them
        int anchorPos = relPath.indexOf('#');
        if (anchorPos != -1) {
            relPath = relPath.left(anchorPos);
        }

        if (relPath.isEmpty()) continue;
        
        // Replace extension with .rst (e.g., .html or .help)
        QFileInfo info(relPath);
        QString rstFile = info.path();
        if (!rstFile.isEmpty() && !rstFile.endsWith("/")) {
            rstFile += "/";
        }
        rstFile += info.baseName() + ".rst";
        if ( rstFile.startsWith("./")) {
            rstFile = rstFile.mid(2);
        }

        QString fullRstPath = sourcePath + rstFile;
        if (!QFile::exists(fullRstPath)) {
            missingCount++;
            QString errorMsg = QString("Not matching rst file for help String: %1 (Expected RST: %2)\n  Context: %3")
                .arg(call.helpString)
                .arg(fullRstPath)
                .arg(call.context);

            ADD_FAILURE_AT(call.file.toStdString().c_str(), call.line) << errorMsg.toStdString();
            
            if (!missingFiles.contains(fullRstPath)) {
                missingFiles.append(fullRstPath);
            }
        }
    }

    if (missingCount > 0) {
        std::cout << "Checked calls: " << allCalls.size() << std::endl;
        std::cout << "Total missing help source files: " << missingCount << std::endl;
        std::cout << "Unique missing RST files: " << missingFiles.size() << std::endl;
        // Optional: FAIL() << "Missing " << missingCount << " help source files.";
    } else {
        std::cout << "All " << allCalls.size() << " show_help() calls have corresponding .rst files." << std::endl;
    }
}
