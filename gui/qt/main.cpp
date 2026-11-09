#include <QApplication>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QByteArray>
#include <QMainWindow>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QTimer>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QProgressBar>
#include <QPainter>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QtMath>
#include "ui_error_catalog.h"
#include "resonet_protocol_v1.h"

struct Sample { QDateTime time; int channel=1; double value=0; QString source="SIMULATOR"; };
class Window final : public QMainWindow {
 Q_OBJECT
 struct Session { QString name,date,source; int count=0; };
 QStackedWidget *stack=nullptr; QComboBox *sourceBox=nullptr,*channelBox=nullptr;
 QLabel *badge=nullptr,*metric=nullptr,*countLabel=nullptr,*sessionLabel=nullptr,*deviceStatusLabel=nullptr;
 QComboBox *serialPortBox=nullptr; QPushButton *serialConnectButton=nullptr;
 QSerialPort serialPort; QTimer serialPollTimer; QByteArray serialRx;
 quint16 serialSequence=0; bool serialPending=false;
 QTableWidget *history=nullptr,*sessionTable=nullptr; QChartView *chartView=nullptr;
 QLineSeries *series=nullptr; QTimer timer; QVector<Sample> samples; QVector<Session> sessions;
 QString source="SIMULATOR",dataDir; QVector<QPushButton*> nav;
 int page=0; bool training=false; int elapsed=0;
 QWidget *pageWidget(QString title,QString sub){auto*w=new QWidget;auto*l=new QVBoxLayout(w);l->setContentsMargins(28,24,28,24);auto*t=new QLabel(title);t->setObjectName("title");auto*s=new QLabel(sub);s->setObjectName("muted");s->setWordWrap(true);l->addWidget(t);l->addWidget(s);return w;}
 QFrame *panel(){auto*w=new QFrame;w->setObjectName("panel");return w;}
 void addPage(QWidget*w){stack->addWidget(w);}
 void reportError(const std::string& code,const QString& technicalDetails=QString()){
  const UiErrorPresentation error=uiErrorPresentation(code);
  QMessageBox box(QMessageBox::Warning,QString("Błąd [%1]").arg(QString::fromUtf8(code.c_str())),
                  QString::fromUtf8(error.user_message),QMessageBox::Ok,this);
  if(!technicalDetails.isEmpty())box.setDetailedText(technicalDetails);
  box.exec();
 }
 void build(){
  auto*root=new QWidget;auto*outer=new QHBoxLayout(root);outer->setContentsMargins(0,0,0,0);outer->setSpacing(0);
  auto*side=new QFrame;side->setObjectName("side");side->setFixedWidth(220);auto*sl=new QVBoxLayout(side);
  auto*brand=new QLabel("◈  ResoNet");brand->setObjectName("brand");sl->addWidget(brand);sl->addWidget(new QLabel("BIOFEEDBACK STUDIO"));
  QStringList names={"Pulpit","Pomiary","Kanały","Trening","Gry","Sesje","Historia","Raporty","Ustawienia"};
  for(int i=0;i<names.size();++i){auto*b=new QPushButton(names[i]);b->setObjectName("nav");b->setCheckable(true);nav<<b;sl->addWidget(b);connect(b,&QPushButton::clicked,this,[this,i]{showPage(i);});}
  sl->addStretch();auto*safe=new QLabel("TRYB BEZPIECZNY\nProtokół v1 nie steruje wyjściami.");safe->setWordWrap(true);safe->setObjectName("safe");sl->addWidget(safe);
  auto*right=new QWidget;auto*rl=new QVBoxLayout(right);rl->setContentsMargins(0,0,0,0);
  auto*top=new QFrame;top->setObjectName("top");auto*tl=new QHBoxLayout(top);tl->addWidget(new QLabel("Pracownia / Biofeedback"));tl->addStretch();
  sourceBox=new QComboBox;sourceBox->addItem("Symulator","SIMULATOR");sourceBox->addItem("Nagranie","RECORDING");sourceBox->addItem("Arduino — USB (diagnostyka)","LIVE_USB");tl->addWidget(sourceBox);
  serialPortBox=new QComboBox;serialPortBox->setMinimumWidth(150);tl->addWidget(serialPortBox);
  serialConnectButton=new QPushButton("Połącz USB");tl->addWidget(serialConnectButton);
  badge=new QLabel("SYMULATOR");badge->setObjectName("badge");tl->addWidget(badge);rl->addWidget(top);
  stack=new QStackedWidget;rl->addWidget(stack,1);outer->addWidget(side);outer->addWidget(right,1);setCentralWidget(root);
  buildDashboard();buildMeasurements();buildChannels();buildTraining();buildGames();buildSessions();buildHistory();buildReports();buildSettings();
  connect(sourceBox,qOverload<int>(&QComboBox::currentIndexChanged),this,[this]{setSource(sourceBox->currentData().toString());});
  connect(serialConnectButton,&QPushButton::clicked,this,&Window::toggleSerialConnection);
  connect(&serialPort,&QSerialPort::readyRead,this,&Window::readSerialData);
  connect(&serialPort,&QSerialPort::errorOccurred,this,[this](QSerialPort::SerialPortError error){
   if(error!=QSerialPort::NoError && serialPort.isOpen() &&
      (error==QSerialPort::ResourceError || error==QSerialPort::DeviceNotFoundError)){
    serialPending=false;serialPollTimer.stop();serialPort.close();serialConnectButton->setText("Połącz USB");
    if(deviceStatusLabel)deviceStatusLabel->setText("USB: utracono połączenie — "+serialPort.errorString());
   }
  });
  serialPollTimer.setInterval(1500);connect(&serialPollTimer,&QTimer::timeout,this,&Window::requestSerialStatus);
  refreshSerialPorts();
  connect(&timer,&QTimer::timeout,this,&Window::tick);timer.start(250);
  setStyleSheet("QMainWindow,QWidget{background:#101722;color:#e8eef7;font-family:'Segoe UI',Arial;font-size:13px}#side{background:#0b111b;border-right:1px solid #29394e}#brand{font-size:23px;font-weight:700;padding:8px}#top{background:#141e2c;padding:8px;border-bottom:1px solid #29394e}#title{font-size:28px;font-weight:700}#muted{color:#91a2b8}#panel{background:#172333;border:1px solid #2a3a50;border-radius:12px;padding:14px}#nav{background:transparent;border:0;text-align:left;padding:12px;color:#aab8ca;border-radius:8px}#nav:checked,#nav:hover{background:#1b2b40;color:white}#badge{background:#193c3b;color:#8de1d1;padding:7px;border-radius:8px;font-weight:700}#safe{background:#2b2520;color:#e7bd8e;padding:10px;border-radius:8px}QPushButton{background:#236c70;border:0;border-radius:7px;padding:9px 14px;color:white}QComboBox{background:#1b293a;padding:7px;border:1px solid #34465e;border-radius:6px}QTableWidget{background:#141f2e;alternate-background-color:#192638;gridline-color:#29394e}QHeaderView::section{background:#202e40;padding:8px;border:0}QProgressBar{height:18px}");
 }
 void buildDashboard(){auto*w=pageWidget("Pulpit","Bieżący podgląd źródła danych i strumienia pomiarowego.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*g=new QGridLayout;
  auto metricCard=[&](QString title,QString value,QLabel**out){auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel(title));auto*v=new QLabel(value);v->setStyleSheet("font-size:28px;font-weight:700;color:#74d6c3");cl->addWidget(v);*out=v;return c;};
  g->addWidget(metricCard("Ostatnia wartość","—",&metric),0,0);g->addWidget(metricCard("Próbki w pamięci","0",&countLabel),0,1);g->addWidget(metricCard("Zapisane sesje","0",&sessionLabel),0,2);
  auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel("Stan systemu"));deviceStatusLabel=new QLabel("Źródło: symulator lokalny\nUSB Arduino: rozłączono\nProtokół: STATUS v1 — tylko odczyt\nWyjścia: sterowanie niedostępne; stan fizycznej blokady niezweryfikowany.");deviceStatusLabel->setWordWrap(true);cl->addWidget(deviceStatusLabel);l->addLayout(g);l->addWidget(c);l->addStretch();addPage(w);}
 void buildMeasurements(){auto*w=pageWidget("Pomiary","Wykres szeregu czasowego. Każda próbka zachowuje oznaczenie źródła.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*bar=new QHBoxLayout;channelBox=new QComboBox;for(int i=1;i<=8;++i)channelBox->addItem(QString("Kanał %1").arg(i),i);connect(channelBox,qOverload<int>(&QComboBox::currentIndexChanged),this,&Window::refreshChart);bar->addWidget(channelBox);bar->addStretch();auto*ex=new QPushButton("Eksportuj CSV");connect(ex,&QPushButton::clicked,this,&Window::exportCsv);bar->addWidget(ex);
  series=new QLineSeries;auto*chart=new QChart;chart->addSeries(series);chart->legend()->hide();chart->setTitle("Sygnał w czasie");auto*x=new QValueAxis;x->setRange(0,120);auto*y=new QValueAxis;y->setRange(0,100);y->setTitleText("Wartość [j.u.]");chart->addAxis(x,Qt::AlignBottom);chart->addAxis(y,Qt::AlignLeft);series->attachAxis(x);series->attachAxis(y);chartView=new QChartView(chart);chartView->setMinimumHeight(420);chartView->setRenderHint(QPainter::Antialiasing);l->addLayout(bar);l->addWidget(chartView,1);addPage(w);}
 void buildChannels(){auto*w=pageWidget("Kanały","Osiem logicznych kanałów prezentacyjnych. Konfiguracja lokalna — bez poleceń do sprzętu.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*g=new QGridLayout;for(int i=1;i<=8;++i){auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel(QString("Kanał %1").arg(i)));cl->addWidget(new QLabel(i%2?"Biofeedback / wejście":"Trening / wizualizacja"));cl->addWidget(new QLabel("Stan: lokalny podgląd"));g->addWidget(c,(i-1)/4,(i-1)%4);}l->addLayout(g);l->addStretch();addPage(w);}
 void buildTraining(){auto*w=pageWidget("Trening","Lokalna sesja demonstracyjna. Nie steruje urządzeniem.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel("Trening koncentracji"));auto*progress=new QProgressBar;progress->setRange(0,100);progress->setObjectName("progress");cl->addWidget(progress);auto*b=new QPushButton("Rozpocznij / zakończ trening");connect(b,&QPushButton::clicked,this,[this,progress]{training=!training;if(training)elapsed=0;else saveSession("Trening koncentracji");progress->setValue(0);});l->addWidget(c);l->addStretch();addPage(w);trainingProgress=progress;}
 QProgressBar*trainingProgress=nullptr;
 void buildGames(){auto*w=pageWidget("Gry","Ćwiczenie reakcyjne. Wynik jest treningowy, nie diagnostyczny.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*c=panel();auto*cl=new QVBoxLayout(c);auto*r=new QLabel("Gotowy");r->setStyleSheet("font-size:25px");auto*b=new QPushButton("Zarejestruj reakcję");connect(b,&QPushButton::clicked,this,[r]{r->setText(QString("Wynik: %1 ms").arg(QRandomGenerator::global()->bounded(220,650)));});cl->addWidget(r);cl->addWidget(b);l->addWidget(c);l->addStretch();addPage(w);}
 void buildSessions(){auto*w=pageWidget("Sesje","Lokalny rejestr zapisany na urządzeniu użytkownika.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());sessionTable=new QTableWidget(0,4);sessionTable->setHorizontalHeaderLabels({"Nazwa","Data","Źródło","Próbki"});sessionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);auto*b=new QPushButton("Zapisz bieżącą sesję");connect(b,&QPushButton::clicked,this,[this]{saveSession("Sesja biofeedback");});l->addWidget(b);l->addWidget(sessionTable,1);addPage(w);}
 void buildHistory(){auto*w=pageWidget("Historia","Ostatnie próbki z aktywnego źródła.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());history=new QTableWidget(0,4);history->setHorizontalHeaderLabels({"Czas","Kanał","Wartość","Źródło"});history->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);l->addWidget(history,1);addPage(w);}
 void buildReports(){auto*w=pageWidget("Raporty","Eksport danych pomiarowych. Raport zawiera czas, kanał, jednostkę i pochodzenie.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel("Eksport obejmuje próbki znajdujące się w pamięci aplikacji. Dane demonstracyjne pozostają oznaczone jako symulowane."));auto*b=new QPushButton("Utwórz raport CSV");connect(b,&QPushButton::clicked,this,&Window::exportCsv);cl->addWidget(b);l->addWidget(c);l->addStretch();addPage(w);}
 void buildSettings(){auto*w=pageWidget("Ustawienia","Źródła, trwałość danych i ograniczenia bezpieczeństwa.");auto*l=qobject_cast<QVBoxLayout*>(w->layout());auto*c=panel();auto*cl=new QVBoxLayout(c);cl->addWidget(new QLabel("Symulator — dane demonstracyjne.\nNagranie — import CSV do analizy lokalnej.\nUrządzenie — protokół v1 jest diagnostyczny i nie udostępnia sterowania wyjściami.\nWyniki nie stanowią samodzielnej diagnozy medycznej."));auto*b=new QPushButton("Wczytaj nagranie CSV");connect(b,&QPushButton::clicked,this,&Window::importCsv);cl->addWidget(b);l->addWidget(c);l->addStretch();addPage(w);}
 void showPage(int i){page=i;stack->setCurrentIndex(i);for(int n=0;n<nav.size();++n)nav[n]->setChecked(n==i);if(i==5)refreshSessions();if(i==6)refreshHistory();}
 void setSource(QString s){source=s;if(s=="LIVE_USB"){timer.stop();badge->setText("ARDUINO USB · DIAGNOSTYKA");if(deviceStatusLabel && !serialPort.isOpen())deviceStatusLabel->setText("USB: wybierz port szeregowy i kliknij Połącz USB.\nSTATUS v1 udostępnia wyłącznie diagnostykę; nie przesyła próbek biologicznych ani poleceń wyjść.");}else{badge->setText(s=="SIMULATOR"?"SYMULATOR":"NAGRANIE");if(s=="SIMULATOR")timer.start(250);else timer.stop();}refreshAll();}
 void refreshSerialPorts(){if(!serialPortBox)return;const QString selected=serialPortBox->currentData().toString();serialPortBox->clear();for(const auto &info:QSerialPortInfo::availablePorts())serialPortBox->addItem(info.portName()+" — "+info.description(),info.portName());const int index=serialPortBox->findData(selected);if(index>=0)serialPortBox->setCurrentIndex(index);}
 void toggleSerialConnection(){if(serialPort.isOpen()){serialPollTimer.stop();serialPending=false;serialPort.close();serialConnectButton->setText("Połącz USB");if(deviceStatusLabel)deviceStatusLabel->setText("USB: rozłączono.");return;}const QString portName=serialPortBox->currentData().toString();if(portName.isEmpty()){refreshSerialPorts();QMessageBox::warning(this,"USB","Nie wykryto portu. Sprawdź przewód i uprawnienia grupy dialout.");return;}serialPort.setPortName(portName);serialPort.setBaudRate(QSerialPort::Baud115200);serialPort.setDataBits(QSerialPort::Data8);serialPort.setParity(QSerialPort::NoParity);serialPort.setStopBits(QSerialPort::OneStop);serialPort.setFlowControl(QSerialPort::NoFlowControl);if(!serialPort.open(QIODevice::ReadWrite)){QMessageBox::warning(this,"USB — błąd połączenia",serialPort.errorString());return;}serialPort.clear(QSerialPort::AllDirections);serialRx.clear();serialPending=false;serialConnectButton->setText("Rozłącz USB");sourceBox->setCurrentIndex(sourceBox->findData("LIVE_USB"));if(deviceStatusLabel)deviceStatusLabel->setText("USB: port otwarty; oczekiwanie na STATUS...");QTimer::singleShot(2200,this,[this]{if(serialPort.isOpen()){requestSerialStatus();serialPollTimer.start();}});}
 void requestSerialStatus(){if(!serialPort.isOpen()||serialPending)return;ResoNetV1::Frame request;request.type=ResoNetV1::kStatusReq;request.sequence=++serialSequence;std::vector<std::uint8_t> bytes;if(!ResoNetV1::encode(request,bytes)){if(deviceStatusLabel)deviceStatusLabel->setText("USB: błąd kodowania zapytania.");return;}serialPending=true;const quint16 expected=serialSequence;const QByteArray frame(reinterpret_cast<const char*>(bytes.data()),static_cast<int>(bytes.size()));if(serialPort.write(frame)!=frame.size()){serialPending=false;if(deviceStatusLabel)deviceStatusLabel->setText("USB: błąd wysyłania zapytania — "+serialPort.errorString());return;}QTimer::singleShot(1800,this,[this,expected]{if(serialPending&&serialSequence==expected){serialPending=false;if(deviceStatusLabel)deviceStatusLabel->setText("USB: brak odpowiedzi STATUS (timeout). Sprawdź firmware, port i prędkość 115200.");}});}
 void readSerialData(){serialRx.append(serialPort.readAll());while(serialRx.size()>=11){const int magic=serialRx.indexOf(QByteArray::fromHex("524e"));if(magic<0){serialRx.clear();return;}if(magic>0)serialRx.remove(0,magic);if(serialRx.size()<11)return;const auto *p=reinterpret_cast<const std::uint8_t*>(serialRx.constData());const std::size_t payloadLength=(static_cast<std::size_t>(p[7])<<8)|p[8];if(payloadLength>ResoNetV1::kMaxPayload){serialRx.remove(0,1);continue;}const int frameLength=static_cast<int>(ResoNetV1::kHeaderSize+payloadLength+2);if(serialRx.size()<frameLength)return;std::vector<std::uint8_t> bytes(static_cast<std::size_t>(frameLength));for(int i=0;i<frameLength;++i)bytes[static_cast<std::size_t>(i)]=static_cast<std::uint8_t>(serialRx.at(i));serialRx.remove(0,frameLength);ResoNetV1::Frame response;if(!ResoNetV1::decode(bytes,response)||response.type!=ResoNetV1::kStatusResp||response.sequence!=serialSequence||!serialPending)continue;serialPending=false;const auto &v=response.payload;if(v.size()!=14||v[0]>5||(((v[1]<<8)|v[2])&~0x0003)!=0||v[11]!=0||v[12]!=0||v[13]>1){if(deviceStatusLabel)deviceStatusLabel->setText("USB: odpowiedź STATUS ma nieprawidłowy format.");continue;}const auto u16=[&v](int i){return (static_cast<unsigned>(v[static_cast<std::size_t>(i)])<<8)|v[static_cast<std::size_t>(i+1)];};const quint32 uptime=(static_cast<quint32>(v[3])<<24)|(static_cast<quint32>(v[4])<<16)|(static_cast<quint32>(v[5])<<8)|v[6];const QStringList states={"UNKNOWN","IDLE","RUNNING","PAUSED","ERROR","LOCKOUT"};const bool tempValid=u16(9)!=0x7FFF;const QString temperature=tempValid?QString::number(static_cast<qint16>(u16(9))/100.0,'f',2)+" °C":"brak zweryfikowanego czujnika";if(deviceStatusLabel)deviceStatusLabel->setText(QString("USB: POŁĄCZONO — %1\nStan firmware: %2; flagi błędów: 0x%3\nCzas pracy: %4 s; wolna pamięć: %5 B\nTemperatura: %6\nCzujniki: brak zweryfikowanych danych; blokada fizyczna wyjść: NIEZWERYFIKOWANA.\nTransport: USB 115200, STATUS v1 — tylko odczyt.").arg(serialPort.portName(),states[static_cast<int>(v[0])]).arg(u16(1),4,16,QChar('0')).arg(uptime).arg(u16(7)).arg(temperature));badge->setText("ARDUINO USB · POŁĄCZONO");}}
 void tick(){if(source!="SIMULATOR")return;Sample s;s.time=QDateTime::currentDateTime();s.channel=1+QRandomGenerator::global()->bounded(8);s.value=48+17*qSin(samples.size()/13.0)+QRandomGenerator::global()->bounded(900)/100.0;samples.append(s);if(samples.size()>2000)samples.remove(0,samples.size()-2000);if(training){++elapsed;if(trainingProgress)trainingProgress->setValue(elapsed%101);}refreshAll();}
 void refreshAll(){if(metric)metric->setText(samples.isEmpty()?"—":QString::number(samples.last().value,'f',1));if(countLabel)countLabel->setText(QString::number(samples.size()));if(sessionLabel)sessionLabel->setText(QString::number(sessions.size()));if(page==1)refreshChart();if(page==6)refreshHistory();}
 void refreshChart(){if(!series||!channelBox)return;series->clear();int ch=channelBox->currentData().toInt(),n=0;for(const auto&s:samples)if(s.channel==ch)series->append(n++,s.value);}
 void refreshHistory(){if(!history)return;history->setRowCount(0);for(int i=samples.size()-1;i>=qMax(0,samples.size()-250);--i){auto s=samples[i];int r=history->rowCount();history->insertRow(r);history->setItem(r,0,new QTableWidgetItem(s.time.toString(Qt::ISODateWithMs)));history->setItem(r,1,new QTableWidgetItem(QString::number(s.channel)));history->setItem(r,2,new QTableWidgetItem(QString::number(s.value,'f',2)));history->setItem(r,3,new QTableWidgetItem(s.source));}}
 void saveSession(QString name){sessions.append({name,QDateTime::currentDateTime().toString(Qt::ISODate),source,static_cast<int>(samples.size())});persist();refreshSessions();refreshAll();}
 void refreshSessions(){if(!sessionTable)return;sessionTable->setRowCount(0);for(const auto&s:sessions){int r=sessionTable->rowCount();sessionTable->insertRow(r);sessionTable->setItem(r,0,new QTableWidgetItem(s.name));sessionTable->setItem(r,1,new QTableWidgetItem(s.date));sessionTable->setItem(r,2,new QTableWidgetItem(s.source));sessionTable->setItem(r,3,new QTableWidgetItem(QString::number(s.count)));}}
 void persist(){QJsonArray a;for(const auto&s:sessions){QJsonObject o;o["name"]=s.name;o["date"]=s.date;o["source"]=s.source;o["count"]=s.count;a.append(o);}QFile f(dataDir+"/sessions.json");if(f.open(QIODevice::WriteOnly|QIODevice::Truncate))f.write(QJsonDocument(a).toJson());}
 void load(){dataDir=QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);QDir().mkpath(dataDir);QFile f(dataDir+"/sessions.json");if(f.open(QIODevice::ReadOnly)){auto a=QJsonDocument::fromJson(f.readAll()).array();for(auto v:a){auto o=v.toObject();sessions.append({o["name"].toString(),o["date"].toString(),o["source"].toString(),o["count"].toInt()});}}}
 void exportCsv(){QString path=QFileDialog::getSaveFileName(this,"Eksport CSV",QDir::homePath()+"/resonet.csv","CSV (*.csv)");if(path.isEmpty())return;QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){reportError("FILE_WRITE_FAILED",f.errorString());return;}QTextStream o(&f);o<<"timestamp,channel,value,unit,source\n";for(const auto&s:samples)o<<s.time.toString(Qt::ISODateWithMs)<<","<<s.channel<<","<<QString::number(s.value,'f',4)<<",j.u.,"<<s.source<<"\n";QMessageBox::information(this,"Eksport","Zapisano "+QString::number(samples.size())+" próbek.");}
 void importCsv(){QString p=QFileDialog::getOpenFileName(this,"Import CSV",QDir::homePath(),"CSV (*.csv)");if(p.isEmpty())return;QFile f(p);if(!f.open(QIODevice::ReadOnly|QIODevice::Text)){reportError("FILE_READ_FAILED",f.errorString());return;}QTextStream in(&f);if(in.readLine().trimmed()!="timestamp,channel,value,unit,source"){reportError("FILE_INVALID_FORMAT","Oczekiwany nagłówek: timestamp,channel,value,unit,source");return;}QVector<Sample> loaded;while(!in.atEnd()){auto x=in.readLine().split(',');if(x.size()!=5)continue;bool a=false,b=false;int ch=x[1].toInt(&a);double val=x[2].toDouble(&b);auto dt=QDateTime::fromString(x[0],Qt::ISODateWithMs);if(a&&b&&ch>=1&&ch<=8&&dt.isValid())loaded.append({dt,ch,val,"RECORDING"});}if(loaded.isEmpty()){reportError("FILE_NO_VALID_RECORDS","Plik nie zawiera poprawnych rekordów próbek.");return;}samples=loaded;source="RECORDING";sourceBox->setCurrentIndex(1);badge->setText("NAGRANIE");refreshAll();}
public:
 Window(){setWindowTitle("ResoNet | Biofeedback Studio");resize(1440,900);setMinimumSize(1100,700);build();load();refreshSessions();}
};
int main(int argc,char**argv){QApplication app(argc,argv);QApplication::setApplicationName("ResoNet Biofeedback Studio");QApplication::setOrganizationName("ResoNet");Window w;w.show();return app.exec();}
#include "main.moc"
