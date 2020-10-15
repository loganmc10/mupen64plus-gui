
#include "oglwindow.h"

void OGLWindow::initializeGL() {
}

void OGLWindow::resizeEvent(QResizeEvent *event) {
    QOpenGLWindow::resizeEvent(event);
    if (timerId) {
        killTimer(timerId);
        timerId = 0;
    }
    timerId = startTimer(500);
    m_width = event->size().width();
    m_height = event->size().height();
}

void OGLWindow::timerEvent(QTimerEvent *te) {
    int size = (m_width << 16) + m_height;
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_VIDEO_SIZE, &size);
    killTimer(te->timerId());
    timerId = 0;
    requestActivate();
}
