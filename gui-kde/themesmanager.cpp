#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "themesmanager.h"
#include "fc_config.h"

// Qt
#include <QApplication>
#include <QDir>
#include <QPalette>
#include <QStyleFactory>
#include <QTextStream>
#include <QDebug>
#include <QPixmapCache>

/* utility */
#include "mem.h"

/* client */
#include "themes_common.h"


// gui-qt
#include "fc_client.h"


#pragma GCC diagnostic pop




using namespace KV;

ThemesManager::ThemesManager()
    : m_Current()
    , m_Default("System")
    , m_Template("%1/themes/gui-kde")
{
}


ThemesManager* ThemesManager::instance() {
    static ThemesManager* manager = new ThemesManager();
    return manager;
}


void ThemesManager::loadTheme(const QString& theme_name, const QString& theme_path) {

    QFile res(theme_path);
    if (!res.open(QFile::ReadOnly | QFile::Text)) {
        qWarning() << "Cannot open" << theme_path
                << ", not changing the current theme" << m_Current;
        return;
    }

    m_Current = theme_name;

    QPixmapCache::clear();
    QTextStream s(&res);
    qApp->setStyleSheet(s.readAll());

}

QStringList ThemesManager::getPaths() const {
    auto dirs = get_data_dirs();
    QStringList paths;
    strvec_iterate(dirs, dir) {
        paths << m_Template.arg(dir);
    } strvec_iterate_end;
    return paths;
}

QStringList ThemesManager::getThemes(const QString &themes_path) const {

    QDir dir;
    dir.setPath(themes_path);

    QStringList subdirs = dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    QStringList themes;
    for (auto subdir: subdirs) {
        QFile res(themes_path + "/" + subdir + "/resource.qss");
        if (res.exists()) {
            themes << subdir;
        }
    }

    // move default theme to head
    QString def_theme(gui_options.gui_qt_default_theme_name);

    if (themes.contains(def_theme)) {
        themes.removeAll(def_theme);
        themes.prepend(def_theme);
    }

    return themes;
}

const QString& ThemesManager::Current() {
    return instance()->m_Current;
}

const QString& ThemesManager::Default() {
    return instance()->m_Default;
}

/*************************************************************************//**
  Loads a kde theme directory/theme_name
*****************************************************************************/
void ThemesManager::LoadTheme(const char *themes_path, const char *theme_name) {
    auto theme_path = QString(themes_path) + "/" + theme_name + "/resource.qss";
    instance()->loadTheme(theme_name, theme_path);
    QPalette pal;
    pal.setBrush(QPalette::Link, QColor(92,170,229));
    pal.setBrush(QPalette::LinkVisited, QColor(54,150,229));
    QApplication::setPalette(pal);
}

/*************************************************************************//**
  Clears a theme (sets default system theme)
*****************************************************************************/
void ThemesManager::ClearTheme() {
    if (!load_theme(ThemesManager::Default().toLatin1().constData())) {
        /* TRANS: No full stop after the URL, could cause confusion. */
        log_fatal(_("No KDE-client theme was found. For instructions on how to "
                    "get one, please visit %s"), WIKI_URL);
        exit(EXIT_FAILURE);
    }
}

/*************************************************************************//**
  Each gui has its own themes directories.

  Returns an array containing these strings and sets array size in count.
  The caller is responsible for freeing the array and the paths.
*****************************************************************************/
char **ThemesManager::GetPaths(int *count) {
    auto paths = instance()->getPaths();

    auto c_paths = new char* [sizeof(char*) * paths.size()];
    int i = 0;
    for (auto p: paths) {
        c_paths[i++] = fc_strdup(p.toLatin1().constData());
    }
    *count = paths.size();
    return c_paths;
}

/*************************************************************************//**
  Return an array of names of usable themes in the given directory.
  Array size is stored in count.
  The caller is responsible for freeing the array and the names
*****************************************************************************/
char **ThemesManager::GetThemes(const char *theme_path, int *count) {
    auto themes = instance()->getThemes(theme_path);

    auto c_themes = new char* [themes.size()];

    int i = 0;
    for (auto t: themes) {
        c_themes[i++] = fc_strdup(t.toLatin1().constData());
    }

    *count = themes.size();
    return c_themes;
}
