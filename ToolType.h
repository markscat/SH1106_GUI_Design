#ifndef TOOLTYPE_H
#define TOOLTYPE_H
#define SelectCopy

enum ToolType {
    Tool_Pen,
    Tool_Line,
    Tool_Rectangle,
    Tool_FilledRectangle,
    Tool_Circle,
#ifdef  SelectCopy

    Tool_SelectCopy,// pushButton_SelectCopy,
    Tool_SelectCut, //pushButton_SelectCut,
    Tool_copy //pushButton_copy
#endif
    //之后可以继续添加 Triangle 等
};

#endif // TOOLTYPE_H
