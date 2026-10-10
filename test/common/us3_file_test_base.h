// Give each test an isolated work directory and restore settings afterward.

#pragma once

#include "qt_test_base.h"
#include "us_settings.h"

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <memory>

class Us3FileTestBase : public QtTestBase
{
protected:
    void SetUp() override
    {
        QtTestBase::SetUp();

        // Remember the process sandbox so TearDown can put it back.
        processWorkRoot_   = US_Settings::workBaseDir();
        processImportDir_  = US_Settings::importDir();
        processTmpDir_     = US_Settings::tmpDir();

        // Live inside the process sandbox rather than beside it, so a leaked
        // directory is still cleaned up when the process sandbox is removed.
        caseDir_ = std::make_unique<QTemporaryDir>(
            processWorkRoot_ + "/case-XXXXXX" );
        ASSERT_TRUE( caseDir_->isValid() )
            << "Could not create a per-test work root under "
            << qPrintable( processWorkRoot_ );

        const QString root      = caseDir_->path();
        const QString imports   = root + "/imports";
        const QString temporary = root + "/tmp";

        ASSERT_TRUE( QDir().mkpath( imports ) );
        ASSERT_TRUE( QDir().mkpath( temporary ) );

        US_Settings::set_workBaseDir( root );
        US_Settings::set_importDir  ( imports );
        US_Settings::set_tmpDir     ( temporary );
    }

    void TearDown() override
    {
        US_Settings::set_workBaseDir( processWorkRoot_ );
        US_Settings::set_importDir  ( processImportDir_ );
        US_Settings::set_tmpDir     ( processTmpDir_ );

        caseDir_.reset();

        QtTestBase::TearDown();
    }

    //! The work root belonging to this test alone.
    QString caseWorkRoot() const { return caseDir_->path(); }

private:
    std::unique_ptr<QTemporaryDir> caseDir_;
    QString                        processWorkRoot_;
    QString                        processImportDir_;
    QString                        processTmpDir_;
};
