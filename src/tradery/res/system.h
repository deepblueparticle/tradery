/*********************************************/
#undef SYSTEM_ID

#define SYSTEM_ID "$(SYSTEM_UUID)"

#pragma message("#systemName=$(SYSTEM_NAME)")
#pragma message("#className=$(SYSTEM_CLASS_NAME)")

class $(SYSTEM_CLASS_NAME) : public BarSystem<$(SYSTEM_CLASS_NAME)> {
 public:
  $(SYSTEM_CLASS_NAME)
  (const std::vector<std::string>* params = 0)
      : BarSystem<$(SYSTEM_CLASS_NAME)>(
            Info("$(SYSTEM_UUID)", "$(SYSTEM_NAME)", "$(SYSTEM_DESCRIPTION)"),
            "$(SYSTEM_DB_ID)") {}

 private:
#include <$(SYSTEM_UUID).h>
};
