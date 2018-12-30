void
entryfunc(int param)
{
	lock();
	lock(2, 3);

	switch(param1) {
		case 0:
			release();
			break;
		case 1:
			release();
			break;
		case 2:
			release();
			break;
	}
}
