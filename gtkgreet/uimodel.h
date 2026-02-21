struct UiModel {
  char *readCommand;
  char *commandList;
};

extern struct UiModel *uimodel;

struct UiModel *create_uimodel();
