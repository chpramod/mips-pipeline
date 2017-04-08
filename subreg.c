
int main(void)
{
	char arr[] = "want100";
	char store_arr[8];
	memset(store_arr, '\0', sizeof(store_arr));
	memcpy(store_arr+2, arr+2, 4);
	printf("memcpy : %s\n", store_arr+2);
	return 123;
}