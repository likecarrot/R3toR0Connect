#include	<Windows.h>
#include    <stdio.h>
#include	"header.h"


/*
* 驱动与应用层通信框架,生成一个通信设备然后应用层用来打开设备进行通信.
*/
int main()
{
	do
	{
		HANDLE hFile = CreateFile(DEVICENAME, GENERIC_ALL, 0,
			NULL, OPEN_EXISTING, 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			printf("打开设备失败.\n");
			break;
		}
		else
		{
			printf("打开设备成功.\n");
		}
		printf("尝试读请求设备.\n");
		char	buffer[32];
		int		retLen = 0;
		BOOL	status = FALSE;
		status=ReadFile(hFile, buffer, sizeof(buffer), &retLen, NULL);
		if (retLen&&status)
		{
			printf("buffer-%s.\n", buffer);
		}
		else {
			printf("buffer==null.\n");
			break;
		}
		printf("尝试写请求设备.\n");
		strcpy(buffer, "ABCDEFGHIJKL");
		status = WriteFile(hFile, buffer, sizeof(buffer), &retLen, NULL);
		if (status&&retLen)
		{
			printf("发送成功.\n");
		}
		else {
			printf("发送失败.\n");
			break;
		}
		CloseHandle(hFile);
	} while (FALSE);

	system("pause");
	return 0;
}