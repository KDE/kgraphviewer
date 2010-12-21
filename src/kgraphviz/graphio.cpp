/*
    Copyright (C) 2010 Kevin Funk <krf@electrostorm.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "graphio.h"
#include "graphio_p.h"

#include <QFile>
#include <QProcess>

#include <KDebug>
#include <KLocale>
#include <KTemporaryFile>

#include "dotgraph.h"
#include "support/dotgrammar.h"
#include "support/dotgraphparsinghelper.h"

#include <boost/spirit/include/classic_assign_actor.hpp>
#include <boost/spirit/include/classic_confix.hpp>
#include <boost/spirit/include/classic_distinct.hpp>
#include <kurl.h>

using namespace boost;
using namespace boost::spirit::classic;

using namespace KGraphViz;

extern KGraphViz::DotGraphParsingHelper* phelper;

const distinct_parser<> keyword_p("0-9a-zA-Z_");

GraphIOPrivate::GraphIOPrivate(QObject* parent)
  : QObject(parent)
  , m_dotGraph(0)
{
  connect(&m_process,
          SIGNAL(finished(int, QProcess::ExitStatus)),
          SLOT(processFinished(int, QProcess::ExitStatus)));
  connect(&m_process,
          SIGNAL(error(QProcess::ProcessError)),
          SLOT(processError(QProcess::ProcessError)));
}

GraphIOPrivate::~GraphIOPrivate()
{
}

void GraphIOPrivate::reset()
{
  if (m_dotGraph != 0) {
    delete m_dotGraph;
    m_dotGraph = 0;
  }
}

QString GraphIOPrivate::toString(QProcess::ProcessError error)
{
  switch (error)
  {
  case QProcess::FailedToStart:
    return i18n("Unable to start process.");
  case QProcess::Crashed:
    return i18n("Process crashed.");
  case QProcess::Timedout:
    return i18n("Process timed out.");
  case QProcess::WriteError:
    return i18n("Was not able to write data to the process.");
  case QProcess::ReadError:
    return i18n("Was not able to read data from the process.");
  default:
    return i18n("Unknown error running the process.");
  }
}

void GraphIOPrivate::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  kDebug() << "Error message:" << toString(m_process.error()) << m_process.errorString();

  // TODO: Better error handling here
  QByteArray result = m_process.readAll();
  if (exitCode != 0) {
    emit error(i18n("Failed to load from dot file: %1", result.data()));
    return;
  }

  result.replace("\\\n", "");
  kDebug() << "String size is:" << result.size();
  kDebug() << "String content is:" << result;
  const std::string content =  result.data();

  if (phelper != 0) {
    delete phelper;
    phelper = 0;
  }

  DotGraph* graph = new DotGraph;
  phelper = new DotGraphParsingHelper;
  phelper->graph = graph;
  phelper->z = 1;
  phelper->maxZ = 1;
  phelper->uniq = 0;

  kDebug() << "Parsing dot";
  const bool parsingResult = parse(content);
  delete phelper;
  phelper = 0;

  if (parsingResult)
  {
    m_dotGraph = graph;
    emit(finished());
  }
  else
  {
    delete graph;
    kError() << "parsing failed";
    emit(error("Parsing failed"));
  }
}

void GraphIOPrivate::processError(QProcess::ProcessError error)
{
  kDebug();
  emit(toString(error));
}

GraphIO::GraphIO(QObject* parent)
  : QObject(parent)
  , d_ptr(new GraphIOPrivate)
{
  Q_D(const GraphIO);
  connect(d, SIGNAL(finished()), SIGNAL(finished()));
  connect(d, SIGNAL(error(QString)), SIGNAL(error(QString)));
}

GraphIO::~GraphIO()
{
  delete d_ptr;
}

DotGraph* GraphIO::readData()
{
  Q_D(GraphIO);

  DotGraph* graph = d->m_dotGraph;
  d->m_dotGraph = 0;
  return graph;
}

void GraphIO::loadFromDotFile(const QString& fileName, const QString& layoutCommand)
{
  Q_D(GraphIO);

  // TODO: Better file handling here
  KUrl url(fileName);
  QString localFile = url.toLocalFile();

  const QString layoutCommandForFile = (layoutCommand.isEmpty()
    ? internalLayoutCommandForFile(fileName)
    : layoutCommand);

  QStringList args;
  args << "-Txdot";
  args << localFile;

  kDebug() << "Running:" << qPrintable(layoutCommandForFile) << qPrintable(localFile);
  d->m_process.start(layoutCommandForFile, args, QIODevice::ReadOnly);
  kDebug() << "Started:" << d->m_process.waitForStarted(3000) << d->m_process.error();
}

void GraphIO::saveToDotFile(const DotGraph* dotGraph, const QString& fileName)
{
  kDebug() << fileName;

  QFile f(fileName);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
  {
    kError() << "Unable to open file for writing:" << fileName;
    return;
  }

  QTextStream stream(&f);
  stream << "digraph \"";
  if (dotGraph->id()!="\"\"")
  {
    stream <<dotGraph->id();
  }
  stream <<"\" {\n";

  stream << "graph [" << *dotGraph <<"]" << endl;

  /// @TODO Subgraph are not represented as needed in DotGraph, so it is not
  /// possible to save them back : to be changed !
//   kDebug() << "writing subgraphs";
  GraphSubgraphMap::const_iterator sit;
  for ( sit = dotGraph->subgraphs().begin();
  sit != dotGraph->subgraphs().end(); ++sit )
  {
    const GraphSubgraph& s = **sit;
    (stream) << s;
  }

  kDebug() << "writing nodes";
  GraphNodeMap::const_iterator nit;
  for ( nit = dotGraph->nodes().begin();
        nit != dotGraph->nodes().end(); ++nit )
  {
    kDebug() << "writing node" << (*nit)->id();
    (stream) << **nit;
  }

  kDebug() << "writing edges";
  GraphEdgeMap::const_iterator eit;
  for ( eit = dotGraph->edges().begin();
        eit != dotGraph->edges().end(); ++eit )
  {
    kDebug() << "writing edge" << (*eit)->id();
    stream << **eit;
  }

  stream << "}\n";

  f.close();
}

void GraphIO::updateDot(const DotGraph* dotGraph)
{
  KTemporaryFile tempFile;
  tempFile.setSuffix(".dot");

  const QString fileName = tempFile.name();
  kDebug() << "Using temporary file:" << fileName;

  saveToDotFile(dotGraph, fileName);
  loadFromDotFile(fileName);
}

QString GraphIO::internalLayoutCommandForFile(const QString& fileName)
{
  QFile file(fileName);

  if (!file.open(QIODevice::ReadOnly)) {
    kWarning() << "Can't test dot file. Will try to use the dot command on the file:" << fileName;
    return "dot"; // -Txdot";
  }

  QByteArray fileContent = file.readAll();
  if (fileContent.isEmpty()) return "";
  std::string s =  fileContent.data();
  std::string cmd = "dot";
  parse(s.c_str(),
        (
          !(keyword_p("strict")) >> (keyword_p("graph")[assign_a(cmd,"dot")])
        ), (space_p|comment_p("/*", "*/")) );

  return QString::fromStdString(cmd); // + " -Txdot" ;
}


#include "graphio.moc"
#include "graphio_p.moc"
