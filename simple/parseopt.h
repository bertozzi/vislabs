struct ArgumentList {
  std::string image_name;                   //!< image file name
  int wait_t;                               //!< waiting time
};

bool ParseInputs(ArgumentList& args, int argc, char **argv);

