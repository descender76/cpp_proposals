variant<rete, error_code> e() noexcept;
variant<reto, error_object> o() noexcept;

inspect(e()) {
	<rete> //...
	<error_code> //...
}
inspect(o()) {
	<reto> //...
	<error_object> //...
}