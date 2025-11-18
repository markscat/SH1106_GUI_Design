#include "oledwidget_Paint.h"

QPoint OLEDWidget::convertToOLED(const QPoint &pos)
{

    // 步骤 1: 计算 OLED 图像在 widget 中居中显示的几何信息
    // [注意] 这部分计算逻辑必须与 paintEvent() 中的完全一致！
    const int scaled_width = OledConfig::DISPLAY_WIDTH * scale;
    const int scaled_height = OledConfig::DISPLAY_HEIGHT * scale;
    const int x_offset = (width() - scaled_width) / 2;
    const int y_offset = (height() - scaled_height) / 2;

    // 步骤 2: 将窗口坐标减去偏移量，得到在缩放后图像上的相对坐标
    int relative_x = pos.x() - x_offset;
    int relative_y = pos.y() - y_offset;


    // 步骤 3: 将相对坐标除以缩放比例，得到最终的 OLED 逻辑坐标
    int oled_x = relative_x / scale;
    int oled_y = relative_y / scale;


    // 步骤 4: [重要] 边界限制 (Clamping)
    // 即使鼠标点击在灰色背景区域（绘图区之外），
    // 我们也应该将坐标限制在有效的 OLED 范围内 (0-127, 0-63)。
    // 这可以防止后续的绘图操作访问到无效的坐标。
    oled_x = std::clamp(oled_x, 0, OledConfig::DISPLAY_WIDTH - 1);
    oled_y = std::clamp(oled_y, 0, OledConfig::DISPLAY_HEIGHT - 1);

    // 步骤 5: 返回计算出的逻辑坐标
    return QPoint(oled_x, oled_y);
}


void OLEDWidget::handleSelectPress(QMouseEvent *event)
{
#ifdef Modefiy_1115
    // 开始新的选择时，清除之前的缓冲区
    m_hasValidBuffer = false;
    m_persistentBuffer = QImage();
#endif
    // 步骤 1: 检查是否是鼠标左键按下的事件
    // 通常，我们只用左键来开始一个新的选区。
    if (event->button() == Qt::LeftButton) {
        // 步骤 2: 开启“正在选择”模式
        // 这个布尔标志位 m_isSelecting 非常重要，
        // 它会告诉 mouseMoveEvent 和 paintEvent 当前正处于选区绘制状态。


        m_isSelecting = true;

        // 步骤 3: 转换坐标并记录选区的“起始点”
        const QPoint oled_pos = convertToOLED(event->pos());
        m_startPoint = oled_pos;

        // 步骤 4: 同时将“结束点”也设置为起始点
        // 因为在刚按下的瞬间，选区是一个 0x0 大小的点。
        m_endPoint = oled_pos;

        // 步骤 5: 清除上一次的旧选区
        // m_selectedRegion 存储的是已经“确定”的选区。
        // 一旦开始一个新的选区操作，旧的就应该作废。
        m_selectedRegion = QRect(); // QRect() 创建一个无效的矩形

        // 步骤 6: 请求重绘
        // 调用 update() 会让 paintEvent 被触发，
        // paintEvent 会根据 m_isSelecting = true 来绘制一个（目前还看不见的）黄色的虚线框。
        update();


        // 步骤 7: 接受事件
        event->accept();
    }
    // 如果是右键或其他按键，我们什么都不做，事件会继续传递
}

void OLEDWidget::handleSelectMove(QMouseEvent *event)
{
    // 步骤 1: 安全检查
    // 确保我们只在“正在选择”的状态下执行此逻辑。
    // 这个检查虽然在 mouseMoveEvent 的调用处已经做过，
    // 但在函数内部再做一次，是一种更安全的防御性编程。
    if (!m_isSelecting) {
        return;
    }

    // 步骤 2: 获取当前鼠标的 OLED 逻辑坐标
    const QPoint oled_pos = convertToOLED(event->pos());

    // 步骤 3: 更新选区的“结束点”
    // m_startPoint 在 handleSelectPress 时已经固定下来，我们不去动它。
    // 我们只需要不断地更新 m_endPoint 即可。
    m_endPoint = oled_pos;

    // 步骤 4: 请求重绘以更新预览
    // 这是整个函数最关键的一步。
    // 调用 update() 会触发 paintEvent。
    // paintEvent 会看到 m_isSelecting 是 true，
    // 然后它会使用最新的 m_startPoint 和 m_endPoint
    // 来绘制一个动态变化的黄色虚线矩形。
    // 这就为用户提供了实时的视觉反馈。
    update();

    // 步骤 5: 接受事件
    event->accept();
}

void OLEDWidget::handleSelectRelease(QMouseEvent *event)
{

    // 步骤 1: 安全检查
    // 确保我们是在“正在选择”的状态下松开【鼠标左键】。
    // 如果不是，或者松开的是右键，则忽略此事件。
    if (!m_isSelecting || event->button() != Qt::LeftButton) {
        return;
    }

    // 步骤 2: 关闭“正在选择”模式
    // 这是关键的状态清理步骤。将开关关闭，
    // mouseMoveEvent 就不会再响应选区逻辑了。
    m_isSelecting = false;

    // 步骤 3: 获取最终的鼠标位置并更新“结束点”
    const QPoint oled_pos = convertToOLED(event->pos());
    m_endPoint = oled_pos;

    // 步骤 4: 计算并“固化”最终的选区矩形
    // 使用 QRect 的构造函数和 .normalized() 来创建一个有效的矩形。
    // .normalized() 能确保矩形的左上角坐标值总是小于右下角坐标值，
    // 即使你是从右下往左上拖动鼠标来创建选区的。
    QRect finalRegion = QRect(m_startPoint, m_endPoint).normalized();

    // 步骤 5: [重要] 有效性检查
    // 如果用户只是点击了一下就松开，或者拖动范围太小，
    // 我们应该认为这是一个无效的选区，直接丢弃它。
    // 这样可以避免后续的复制等操作处理一个 1x1 或 0x0 的无效区域。

    if (finalRegion.width() < 2 || finalRegion.height() < 2) {

        // 如果区域太小，我们就创建一个无效的 QRect，相当于清除了选区。
        m_selectedRegion = QRect();


    } else {

        // 如果区域有效，就将其保存在 m_selectedRegion 成员变量中。
        // m_selectedRegion 现在存储了这次操作的最终成果。
        m_selectedRegion = finalRegion;
    }

    // 步骤 6: 请求最后一次重绘
    // 这次 update() 调用有两个目的：
    // 1. 因为 m_isSelecting 已经是 false，paintEvent 将不再根据 start/endPoint
    //    绘制动态的虚线框，清除了“过程”中的预览。
    // 2. 如果我们得到了一个有效的 m_selectedRegion，paintEvent 会根据它
    //    绘制一个“固化”的黄色虚线框，向用户展示最终的选区结果。
    // 3. 如果选区无效，m_selectedRegion 为空，paintEvent 将不会画任何框。
    update();

    event->accept();
}
