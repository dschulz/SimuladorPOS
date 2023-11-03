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
#include <QRandomGenerator>


using namespace Qt::StringLiterals;

namespace util {

void delay(int ms) {
    qDebug().noquote().nospace() << " > Simulando delay de " << ms << "ms";
    QThread::msleep(ms);
    qDebug().noquote().nospace() << " > Continuando...";
}

int randomInt(int lbound, int hbound) {
  QRandomGenerator *rng = QRandomGenerator::global();

  return rng->bounded(lbound, hbound);
}

quint64 randomLong(quint64 lbound, quint64 hbound) {
  QRandomGenerator *rng = QRandomGenerator::global();

  return rng->bounded(lbound, hbound);
}

} // namespace util

namespace endpoint {

static constexpr auto eco = "/pos/eco";
static constexpr auto ventaUx = "/pos/venta-ux";
static constexpr auto credito = "/pos/venta/credito";
static constexpr auto debito = "/pos/venta/debito";
static constexpr auto descuento = "/pos/descuento";
static constexpr auto ventaQr = "/pos/venta-qr";
static constexpr auto ventaCanje = "/pos/venta-canje";
static constexpr auto ventaBilletera = "/pos/venta-billetera";

static constexpr auto listarIssuers = "/issuers/";
static constexpr auto listarBilleteras = "/billeteras/";

} // namespace endpoint

namespace server {

static constexpr auto default_address = "localhost";
static constexpr auto default_port = 3000;

QUrl formatUrl(const QString &route, const QString &ip = default_address,
               int port = default_port) noexcept {
  static const auto fmt = QString{"http://%1:%2/%3"};
    return fmt.arg(ip, QString::number(port), route);
}

} // namespace server



static const auto POST = QHttpServerRequest::Method::Post;
static const auto GET = QHttpServerRequest::Method::Get;

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

static QJsonObject makeErrorResponse(const QString error,
                                     const QString message,
                                     int statusCode) {
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
            auto delay = 8; // 10 + (val * 10);
            util::delay(delay);
            qDebug().noquote().nospace()
                << "Respondiendo " << val << " despues de " << delay << "ms";

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

    auto status = QHttpServerResponder::StatusCode::Ok;

    if (!json)
      return QHttpServerResponse(QHttpServerResponder::StatusCode::BadRequest);


//    if(ventaUxInputInvalido(json.value())){
//      auto malformed =
//          makeErrorResponse("Solicitud mal formada",
//                            "La solicitud contiene datos mal formados", 401);
//      return QHttpServerResponse(malformed,
//                               QHttpServerResponder::StatusCode::BadRequest);
//    }


    auto req = json.value();

    auto facturaNro = req.value("facturaNro").toInteger();
    auto cuotas = req.value("cuotas").toInt();
    auto plan = req.value("plan").toInt();

    qDebug().noquote() << "\n\nFactura Numero: " << facturaNro << "\n\n";

    if (facturaNro < 1 || facturaNro > 99999999999) {
      status = QHttpServerResponder::StatusCode::NotAcceptable;
      auto eResp = makeErrorResponse("Bad request",
                                     "Número de factura inválido", (int)status);

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int)status << "]\n"
          << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

      return QHttpServerResponse(eResp, status);
    }

    if (cuotas < 0 || cuotas > 99 || plan < 0 || plan > 1) {
      status = QHttpServerResponder::StatusCode::NotAcceptable;
      auto eResp = makeErrorResponse(
          "Bad request", "Combinación inválida de Cuotas/Plan", (int)status);

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int)status << "]\n"
          << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

      return QHttpServerResponse(eResp, status);
    }

    auto delay = 1500;

    util::delay(delay);

    QJsonObject nsuBin;
    nsuBin["nsu"] = "UX" + QString::number(util::randomInt(1, 9999999));
    nsuBin["bin"] = "UX" + QString::number(util::randomInt(1, 999999));

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << "\n"
        << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
        << "=> \n"
        << QJsonDocument(nsuBin).toJson(QJsonDocument::Indented) << "\n";

    util::delay(1500);

    return QHttpServerResponse(nsuBin, status);
  });
}

void handleVentaCredito(QHttpServer &httpServer,
                        QHttpServerRequest::Method method,
                        const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());
      auto status =  QHttpServerResponder::StatusCode::Ok;

      if (!json){
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

          qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << request.body() << Qt::endl
              << "==> [" << (int)status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }


      auto req = json.value();

      auto facturaNro = req.value("facturaNro").toInteger();
      auto cuotas = req.value("cuotas").toInt();
      auto plan = req.value("plan").toInt();

      if ( facturaNro < 1 || facturaNro > 99999999999) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "Número de factura inválido", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      if ( cuotas <0 || cuotas>99 || plan<0 || plan>1) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "Combinación inválida de Cuotas/Plan", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      auto delay = 1500;

      util::delay(delay);


      QJsonObject nsuBin;
      nsuBin["nsu"] = QString::number(util::randomInt(1, 9999999));
      nsuBin["bin"] = QString::number(util::randomInt(1, 999999));

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int) status << "]\n" << QJsonDocument(nsuBin).toJson(QJsonDocument::Indented) << "\n";


      return QHttpServerResponse(nsuBin, status);
  });
}

void handleVentaDebito(QHttpServer &httpServer,
                       QHttpServerRequest::Method method, QByteArray path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());
      auto status =  QHttpServerResponder::StatusCode::Ok;

      if (!json){
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

          qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << request.body() << Qt::endl
              << "==> [" << (int)status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }


      auto req = json.value();

      auto facturaNro = req.value("facturaNro").toInteger();

      if ( facturaNro<1 || facturaNro > 99999999999) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "Número de factura inválido", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }


      QJsonObject nsuBin;
      nsuBin["nsu"] = QString::number(util::randomInt(1, 9999999));
      nsuBin["bin"] = QString::number(util::randomInt(1, 999999));

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int) status << "]\n" << QJsonDocument(nsuBin).toJson(QJsonDocument::Indented) << "\n";


      return QHttpServerResponse(nsuBin, status);
  });
}

void handleMontoDescuento(QHttpServer &httpServer,
                          QHttpServerRequest::Method method,
                          const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
    const std::optional<QJsonObject> json =
        byteArrayToJsonObject(request.body());

    auto status =  QHttpServerResponder::StatusCode::Ok;

    if (!json){
      status =  QHttpServerResponder::StatusCode::BadRequest;
        auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

        qDebug().noquote().nospace()
            << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
            << request.body() << Qt::endl
            << "==> [" << (int)status << "]\n"
            << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

        return QHttpServerResponse(eResp, status);
    }

    auto req = json.value();

    auto nsu = req.value("nsu").toString();
    auto bin = req.value("bin").toString();
    auto monto = req.value("monto").toInteger();

    if (nsu.trimmed().isEmpty()|| bin.trimmed().isEmpty() || monto < 1) {
        status =  QHttpServerResponder::StatusCode::NotAcceptable;
        auto eResp =
            makeErrorResponse("Bad request", "NSU o BIN o MONTO inválido", (int) status);

        qDebug().noquote().nospace()
            << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
            << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
            << "==> [" << (int) status << "]\n"
            << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

        return QHttpServerResponse(eResp, status);
    }

    /*
     * Espero que los errores transaccionales
     * no se respondan en este nivel de abstracción
     * ESTO ES SOLO UNA PRUEBA
     */
    if (monto > 1000000) {
        status =  QHttpServerResponder::StatusCode::BadRequest;
        auto eResp =
            makeErrorResponse("Bad request", "Saldo insuficiente", (int) status);

            qDebug().noquote().nospace()
            << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
            << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
            << "==> [" << (int) status << "]\n"
            << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

        return QHttpServerResponse(eResp, status);
    }

    QJsonObject resp;
    resp["codigoAutorizacion"] = QString::number(util::randomInt(1,999999));
    resp["codigoComercio"] = QString::number(util::randomLong(1,9999999999));
    resp["issuerId"] = "ZZ";
    resp["mensajeDisplay"] = "APROBADA";
    resp["montoVuelto"] = util::randomInt(0, 500000);
    resp["saldo"] = util::randomInt(1, 500000000);
    resp["nombreCliente"] = "Nombre de Alguien";
    resp["pan"] = util::randomInt(1,9999);
    resp["nombreTarjeta"] = "VISA ZZZZZZZ";
    resp["nroBoleta"] = QString::number(util::randomLong(1,9999999999));

    qDebug().noquote().nospace()
        << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
        << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
        << "==> [" << (int) status << "]\n" << QJsonDocument(resp).toJson(QJsonDocument::Indented) << "\n";

    return QHttpServerResponse(resp, status);
  });
}

void handleVentaQr(QHttpServer &httpServer, QHttpServerRequest::Method method,
                   const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {

      const std::optional<QJsonObject> json =
          byteArrayToJsonObject(request.body());

      auto status =  QHttpServerResponder::StatusCode::Ok;

      if (!json){
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

                       qDebug().noquote().nospace()
                       << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
                       << request.body() << Qt::endl
                       << "==> [" << (int)status << "]\n"
                       << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      auto req = json.value();

      auto facturaNro = req.value("facturaNro").toInteger();
      auto monto = req.value("monto").toInteger();

      if (monto < 10 || facturaNro < 10) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "NÚMERO DE FACTURA o MONTO inválido", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      /*
     * Espero que los errores transaccionales
     * no se respondan en este nivel de abstracción
     * ESTO ES SOLO UNA PRUEBA
     */
      if (monto > 1000000) {
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp =
              makeErrorResponse("Bad request", "Saldo insuficiente", (int) status);

          qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      QJsonObject resp;
      resp["codigoAutorizacion"] = QString::number(util::randomInt(1,999999));
      resp["codigoComercio"] = QString::number(util::randomLong(1,9999999999));
      resp["issuerId"] = "ZZ";
      resp["mensajeDisplay"] = "APROBADA (QR)";
      resp["montoVuelto"] = util::randomInt(0, 500000);
      resp["saldo"] = util::randomInt(1, 500000000);
      resp["nombreCliente"] = "Nombre de Alguien";
      resp["pan"] = util::randomInt(1,9999);
      resp["nombreTarjeta"] = "VISA ZZZZZZZ";
      resp["nroBoleta"] = QString::number(util::randomLong(1,9999999999));

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int) status << "]\n" << QJsonDocument(resp).toJson(QJsonDocument::Indented) << "\n";

      return QHttpServerResponse(resp, status);
  });
}

void handleVentaCanje(QHttpServer &httpServer,
                      QHttpServerRequest::Method method,
                      const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
      const std::optional<QJsonObject> json =
          byteArrayToJsonObject(request.body());

      auto status =  QHttpServerResponder::StatusCode::Ok;

      if (!json){
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

                       qDebug().noquote().nospace()
                       << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
                       << request.body() << Qt::endl
                       << "==> [" << (int)status << "]\n"
                       << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      auto req = json.value();

      auto facturaNro = req.value("facturaNro").toInteger();
      auto monto = req.value("monto").toInteger();

      if (monto < 10 || facturaNro < 10) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "NÚMERO DE FACTURA o MONTO inválido", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      /*
     * Espero que los errores transaccionales
     * no se respondan en este nivel de abstracción
     * ESTO ES SOLO UNA PRUEBA
     */
      if (monto > 1000000) {
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp =
              makeErrorResponse("Bad request", "Saldo insuficiente", (int) status);

          qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      QJsonObject resp;
      resp["codigoAutorizacion"] = QString::number(util::randomInt(1,999999));
      resp["codigoComercio"] = QString::number(util::randomLong(1,9999999999));
      resp["issuerId"] = "ZZ";
      resp["mensajeDisplay"] = "APROBADA (CANJE)";
      resp["montoVuelto"] = util::randomInt(0, 500000);
      resp["saldo"] = util::randomInt(1, 500000000);
      resp["nombreCliente"] = "Nombre de Alguien";
      resp["pan"] = util::randomInt(1,9999);
      resp["nombreTarjeta"] = "VISA ZZZZZZZ";
      resp["nroBoleta"] = QString::number(util::randomLong(1,9999999999));

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int) status << "]\n" << QJsonDocument(resp).toJson(QJsonDocument::Indented) << "\n";

      return QHttpServerResponse(resp, status);
  });
}

void handleVentaBilletera(QHttpServer &httpServer,
                          QHttpServerRequest::Method method,
                          const QByteArray &path) {
  httpServer.route(path, method, [](const QHttpServerRequest &request) {
      const std::optional<QJsonObject> json =
          byteArrayToJsonObject(request.body());

      auto status =  QHttpServerResponder::StatusCode::Ok;

      if (!json){
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp = makeErrorResponse("Bad request", "JSON inválido", (int) status);

                       qDebug().noquote().nospace()
                       << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
                       << request.body() << Qt::endl
                       << "==> [" << (int)status << "]\n"
                       << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      auto req = json.value();

      auto facturaNro = req.value("facturaNro").toInteger();
      auto monto = req.value("monto").toInteger();

      if (monto < 10 || facturaNro < 10) {
          status =  QHttpServerResponder::StatusCode::NotAcceptable;
          auto eResp =
              makeErrorResponse("Bad request", "NÚMERO DE FACTURA o MONTO inválido", (int) status);

              qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      /*
     * Espero que los errores transaccionales
     * no se respondan en este nivel de abstracción
     * ESTO ES SOLO UNA PRUEBA
     */
      if (monto > 1000000) {
          status =  QHttpServerResponder::StatusCode::BadRequest;
          auto eResp =
              makeErrorResponse("Bad request", "Saldo insuficiente", (int) status);

          qDebug().noquote().nospace()
              << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
              << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
              << "==> [" << (int) status << "]\n"
              << QJsonDocument(eResp).toJson(QJsonDocument::Indented) << "\n";

          return QHttpServerResponse(eResp, status);
      }

      QJsonObject resp;
      resp["codigoAutorizacion"] = QString::number(util::randomInt(1,999999));
      resp["codigoComercio"] = QString::number(util::randomLong(1,9999999999));
      resp["issuerId"] = "ZZ";
      resp["mensajeDisplay"] = "APROBADA (BILLETERA)";
      resp["montoVuelto"] = util::randomInt(0, 500000);
      resp["saldo"] = util::randomInt(1, 500000000);
      resp["nombreCliente"] = "Nombre de Alguien";
      resp["pan"] = util::randomInt(1,9999);
      resp["nombreTarjeta"] = "VISA ZZZZZZZ";
      resp["nroBoleta"] = QString::number(util::randomLong(1,9999999999));

      qDebug().noquote().nospace()
          << request.url().toDisplayString(QUrl::RemoveQuery) << " <==\n"
          << QJsonDocument(req).toJson(QJsonDocument::JsonFormat::Indented)
          << "==> [" << (int) status << "]\n" << QJsonDocument(resp).toJson(QJsonDocument::Indented) << "\n";

      return QHttpServerResponse(resp, status);
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
  handleEco(httpServer, POST, endpoint::eco);                       // OK
  handleVentaUx(httpServer, POST, endpoint::ventaUx);               // OK
  handleVentaCredito(httpServer, POST, endpoint::credito);          // OK
  handleVentaDebito(httpServer, POST, endpoint::debito);            // OK
  handleMontoDescuento(httpServer, POST, endpoint::descuento);      // OK
  handleVentaQr(httpServer, POST, endpoint::ventaQr);               // OK
  handleVentaCanje(httpServer, POST, endpoint::ventaCanje);         // OK
  handleVentaBilletera(httpServer, POST, endpoint::ventaBilletera); // OK

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
