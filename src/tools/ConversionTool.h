#ifndef CONVERSIONTOOL_H
#define CONVERSIONTOOL_H

namespace freewb::tools
{

int txt2mb(char *txtPath, char *mbPath, int *HZcount);
int mb2txt(char *txtPath, char *mbPath, int *HZcount);

} // namespace freewb::tools

#endif // CONVERSIONTOOL_H
