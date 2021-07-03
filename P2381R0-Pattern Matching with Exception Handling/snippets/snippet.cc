inspect (p) {
	[0, 0]: std::cout << "on origin";
	[0, y]: std::cout << "on y-axis";
	[x, 0]: std::cout << "on x-axis";
	[x, y]: std::cout << x << ',' << y;
}
