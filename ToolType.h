#ifndef TOOLTYPE_H
#define TOOLTYPE_H
#define SelectCopy

enum ToolType {
    Tool_Pen,                           //1
    Tool_Line,                          //2
    Tool_Rectangle,                     //3
    Tool_FilledRectangle,               //4
    Tool_Circle,

    Tool_Select,// pushButton_Select,     5
    Tool_Copy,// pushButton_Copy,         6
    Tool_Cut, //pushButton_Cut,           7
    Tool_Paste//pushButton_paste          8

    //之后可以继续添加 Triangle 等
};

#endif // TOOLTYPE_H

//SelectCopy
