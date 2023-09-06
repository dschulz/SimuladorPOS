#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QHttpServer>
#include <QHttpServerResponse>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QtHttpServer/QHttpServerResponse>

using namespace Qt::StringLiterals;

namespace endpoint {

static constexpr auto eco = "/pos/eco";
static constexpr auto ventaUx = "/pos/venta-ux";               /// PEND
static constexpr auto credito = "/pos/venta/credito";          /// PEND
static constexpr auto debito = "/pos/venta/debito";            /// PEND
static constexpr auto descuento = "/pos/descuento";            /// PEND
static constexpr auto ventaQr = "/pos/venta-qr";               /// PEND
static constexpr auto ventaCanje = "/pos/venta-canje";         /// PEND
static constexpr auto ventaBilletera = "/pos/venta-billetera"; /// PEND

static constexpr auto listarIssuers = "/issuers/";
static constexpr auto listarBilleteras = "/billeteras/";

} // namespace endpoint

namespace server {

static constexpr auto default_address = "localhost";
static constexpr auto default_port = "3000";

QUrl formatUrl(const QString &route, const QString &ip = default_address,
               const QString &port = default_port) noexcept {
  static const auto fmt = QString{"http://%1:%2/%3"};
  return fmt.arg(ip, port, route);
}

} // namespace server

static const QHttpServerRequest::Method POST = QHttpServerRequest::Method::Post;
static const QHttpServerRequest::Method GET = QHttpServerRequest::Method::Get;

static inline QString host(const QHttpServerRequest &request) {
  return QString::fromLatin1(request.value("Host"));
}

static std::optional<QJsonObject> byteArrayToJsonObject(const QByteArray &arr) {
  QJsonParseError err;
  const auto json = QJsonDocument::fromJson(arr, &err);
  if (err.error || !json.isObject())
    return std::nullopt;
  return json.object();
}

static QJsonObject makeErrorResponse(const QString error, const QString message,
                                     qint16 statusCode) {
  return QJsonObject{
      {"statusCode", statusCode}, {"error", error}, {"message", message}};
}


void handleIndex(QHttpServer &httpServer, QHttpServerRequest::Method method,
                 QByteArray path) {
  httpServer.route(path, method, []() {
    return "<head><meta charset=\"UTF-8\"></head><h1>Simulador "
           "POS</h1><p>Basado en la especificación del POS de Bancard.</p>\n";
  });
}

void handleEco(QHttpServer &httpServer, QHttpServerRequest::Method method,
               const QByteArray &path) {

  httpServer.route(
      path, QHttpServerRequest::Method::Post,
      [](const QHttpServerRequest &request) {
        QByteArray body = request.body();

        QJsonParseError error;
        auto parsedDocument = QJsonDocument::fromJson(body, &error);

        qInfo().noquote().nospace()
            << "Eco: " << parsedDocument.toJson(QJsonDocument::Compact);

        if (error.error || !parsedDocument.isObject()) {
          return QHttpServerResponse(
              makeErrorResponse(
                  "Bad request",
                  error.errorString() + ": JSON con formato erróneo", 400),
              QHttpServerResponder::StatusCode::BadRequest);
        }

        auto obj = parsedDocument.object();

        if (const QJsonValue v = obj["eco"]; v.isDouble()) {
          int val = (v.toInt());
          if (val >= 0 && val < 100) {

            /// Simular delay artificial
                        if (val % 2 == 0) {
                          auto delay = 100 + (val * 10);
                          qDebug() << "pausa de " << delay << "ms";
                          QThread::msleep(delay);
                          qDebug() << "respondiendo "<< val << " despues de "
                          << delay << "ms";
                        }

            return QHttpServerResponse(parsedDocument.object(),
                                       QHttpServerResponder::StatusCode::Ok);
          } else {
            return QHttpServerResponse(
                makeErrorResponse(
                    "Bad request",
                    QString("Valor fuera del rango admitido [%1]: %2")
                        .arg("1-99")
                        .arg(val),
                    400),
                QHttpServerResponder::StatusCode::BadRequest);
          }
        }

        return QHttpServerResponse(
            makeErrorResponse("Solicitud mal formada",
                              "Lo que recibí es basura", 400),
            QHttpServerResponder::StatusCode::BadRequest);
      });
}

void handleVentaUx(QHttpServer &httpServer, QHttpServerRequest::Method method,
                   const QByteArray &path) {

  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

    /*
    if(ventaUxInputInvalido(json.value()))
    auto malformed = makeErrorResponse(
        "Solicitud mal formada", "La solicitud contiene datos mal formados",
    401); return QHttpServerResponse(malformed,
                               QHttpServerResponder::StatusCode::BadRequest);
    */

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleVentaCredito(QHttpServer &httpServer,
                        QHttpServerRequest::Method method,
                        const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleVentaDebito(QHttpServer &httpServer,
                       QHttpServerRequest::Method method, QByteArray path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleMontoDescuento(QHttpServer &httpServer,
                          QHttpServerRequest::Method method,
                          const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleVentaQr(QHttpServer &httpServer, QHttpServerRequest::Method method,
                   const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleVentaCanje(QHttpServer &httpServer,
                      QHttpServerRequest::Method method,
                      const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleVentaBilletera(QHttpServer &httpServer,
                          QHttpServerRequest::Method method,
                          const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);

//    auto malformed = makeErrorResponse(
//        "Solicitud mal formada", "Solicitud contiene datos mal formados", 401);
//    return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);

    /// CONTINUAR ACA
    auto payload = json.value();

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(payload).toJson(QJsonDocument::JsonFormat::Indented)
        << "\n";

    return QHttpServerResponse(payload, QHttpServerResponder::StatusCode::Ok);
  });
}

void handleListarIssuers(QHttpServer &httpServer,
                         QHttpServerRequest::Method method,
                         const QByteArray &path) {

  httpServer.route(path, method, []() {
    QJsonArray lista;

    lista << QJsonObject{
        {"Marca", "CABAL"}, {"Tipo", "Crédito"}, {"IssuerID", "CB"}};
    lista << QJsonObject{
        {"Marca", "CREDIFIELCO"}, {"Tipo", "Crédito"}, {"IssuerID", "CC"}};
    lista << QJsonObject{
        {"Marca", "CARTA CLAVE"}, {"Tipo", "Crédito"}, {"IssuerID", "CL"}};
    lista << QJsonObject{
        {"Marca", "PANAL"}, {"Tipo", "Crédito"}, {"IssuerID", "CP"}};
    lista << QJsonObject{
        {"Marca", "DINERS"}, {"Tipo", "Crédito"}, {"IssuerID", "DC"}};
    lista << QJsonObject{
        {"Marca", "INFONET"}, {"Tipo", "Débito"}, {"IssuerID", "ID"}};
    lista << QJsonObject{
        {"Marca", "MASTERCARD"}, {"Tipo", "Crédito"}, {"IssuerID", "MC"}};
    lista << QJsonObject{
        {"Marca", "MASTERCARD"}, {"Tipo", "Débito"}, {"IssuerID", "MD"}};
    lista << QJsonObject{
        {"Marca", "CREDICARD"}, {"Tipo", "Crédito"}, {"IssuerID", "PC"}};
    lista << QJsonObject{
        {"Marca", "UNICA"}, {"Tipo", "Débito"}, {"IssuerID", "UD"}};
    lista << QJsonObject{
        {"Marca", "VISA"}, {"Tipo", "Crédito"}, {"IssuerID", "VC"}};
    lista << QJsonObject{
        {"Marca", "VISA"}, {"Tipo", "Débito"}, {"IssuerID", "VD"}};
    lista << QJsonObject{
        {"Marca", "TARJETA DEBITO"}, {"Tipo", "Débito"}, {"IssuerID", "TD"}};
    lista << QJsonObject{
        {"Marca", "TARJETA CREDITO"}, {"Tipo", "Crédito"}, {"IssuerID", "TC"}};
    lista << QJsonObject{
        {"Marca", "DEBITO EN CUENTA"}, {"Tipo", "Débito"}, {"IssuerID", "CD"}};
    lista << QJsonObject{
        {"Marca", "AMERICAN EXPRESS"}, {"Tipo", "Crédito"}, {"IssuerID", "AC"}};
    lista << QJsonObject{
        {"Marca", "BANCARD"}, {"Tipo", "Crédito"}, {"IssuerID", "BC"}};

    return lista;
  });
}

void handleListarBilleteras(QHttpServer &httpServer,
                            QHttpServerRequest::Method method,
                            const QByteArray &path) {
  httpServer.route(path, method, []() {
    QJsonArray lista;

    lista << QJsonObject{
        {"Marca", "ZIMPLE"}, {"CodigoBilletera", "ZIM"}, {"IssuerID", "ZM"}};
    lista << QJsonObject{{"Marca", "Paraguayo Japonesa"},
                         {"CodigoBilletera", "WPJ"},
                         {"IssuerID", "PJ"}};
    lista << QJsonObject{
        {"Marca", "VISION"}, {"CodigoBilletera", "VBV"}, {"IssuerID", "VB"}};
    lista << QJsonObject{{"Marca", "Personal-Itau"},
                         {"CodigoBilletera", "BPI"},
                         {"IssuerID", "PI"}};
    lista << QJsonObject{{"Marca", "Billetera Viru"},
                         {"CodigoBilletera", "BBF"},
                         {"IssuerID", "BF"}};

    return lista;
  });
}

int main(int argc, char *argv[]) {
  QCoreApplication a(argc, argv);

  QHttpServer httpServer;

  handleIndex(httpServer, GET, "/");

  // endpoints
  handleEco(httpServer, POST, endpoint::eco);                       //  OK
  handleVentaUx(httpServer, POST, endpoint::ventaUx);               /// PEND
  handleVentaCredito(httpServer, POST, endpoint::credito);          /// PEND
  handleVentaDebito(httpServer, POST, endpoint::debito);            /// PEND
  handleMontoDescuento(httpServer, POST, endpoint::descuento);      /// PEND
  handleVentaQr(httpServer, POST, endpoint::ventaQr);               /// PEND
  handleVentaCanje(httpServer, POST, endpoint::ventaCanje);         /// PEND
  handleVentaBilletera(httpServer, POST, endpoint::ventaBilletera); /// PEND

  // misc - listados
  handleListarIssuers(httpServer, GET, endpoint::listarIssuers);       //  OK
  handleListarBilleteras(httpServer, GET, endpoint::listarBilleteras); //  OK

  httpServer.afterRequest([](QHttpServerResponse &&resp) {
    resp.setHeader("Server", "SimuladorPOS");
    resp.setHeader("Autor", "Diego Schulz");

    return std::move(resp);
  });

  const auto port = httpServer.listen(QHostAddress::Any, 3000);
  if (!port) {
    qWarning().noquote().nospace() << QCoreApplication::translate(
        "SimuladorPOS", "Error: no se pudo escuchar en el puerto 3000, "
                        "posiblemente ya hay otro proceso pegado al puerto.");
    return -1;
  }

  qInfo().noquote() << QCoreApplication::translate(
                           "SimuladorPOS", "Escuchando en http://127.0.0.1:%1/"
                                           "\n(Presiona CTRL+C para terminar)")
                           .arg(port);

  return a.exec();
}



/*


void ejemplos(QHttpServer &httpServer) {

  httpServer.route("/", []() { return "Hello world"; });

  httpServer.route("/query", [](const QHttpServerRequest &request) {
    return host(request) + u"/query/"_s;
  });

  httpServer.route("/query/", [](qint32 id, const QHttpServerRequest &request) {
    return u"%1/query/%2"_s.arg(host(request)).arg(id);
  });

  httpServer.route("/query/<arg>/log",
                   [](qint32 id, const QHttpServerRequest &request) {
                     return u"%1/query/%2/log"_s.arg(host(request)).arg(id);
                   });

  httpServer.route("/query/<arg>/log/", [](qint32 id, float threshold,
                                           const QHttpServerRequest &request) {
    return u"%1/query/%2/log/%3"_s.arg(host(request)).arg(id).arg(threshold);
  });

  httpServer.route("/user/", [](const qint32 id) {
    return u"User "_s + QString::number(id);
  });

  httpServer.route("/user/<arg>/detail",
                   [](const qint32 id) { return u"User %1 detail"_s.arg(id); });

  httpServer.route("/user/<arg>/detail/",
                   [](const qint32 id, const qint32 year) {
                     return u"User %1 detail year - %2"_s.arg(id).arg(year);
                   });

  httpServer.route("/json/", [] {
    return QJsonObject{{{"key1", "1"}, {"key2", "2"}, {"key3", "3"}}};
  });

  httpServer.route("/assets/<arg>", [](const QUrl &url) {
    return QHttpServerResponse::fromFile(u":/assets/"_s + url.path());
  });

  httpServer.route("/remote_address", [](const QHttpServerRequest &request) {
    return request.remoteAddress().toString();
  });

  // Basic authentication example (RFC 7617)
  httpServer.route("/auth", [](const QHttpServerRequest &request) {
    auto auth = request.value("authorization").simplified();

    if (auth.size() > 6 && auth.first(6).toLower() == "basic ") {
      auto token = auth.sliced(6);
      auto userPass = QByteArray::fromBase64(token);

      if (auto colon = userPass.indexOf(':'); colon > 0) {
        auto userId = userPass.first(colon);
        auto password = userPass.sliced(colon + 1);

        if (userId == "dschulz" && password == "dsds")
          return QHttpServerResponse("text/plain", "Success\n");
      }
    }
    QHttpServerResponse response("text/plain", "Authentication required\n",
                                 QHttpServerResponse::StatusCode::Unauthorized);
    response.setHeader("WWW-Authenticate",
                       R"(Basic realm="SimuladorPOS", charset="UTF-8")");
    return response;
  });
}

*/
