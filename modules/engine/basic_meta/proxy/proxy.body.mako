<%record_proxy_data=record.generator_data["proxy"]%>\
/*content*/
void* self = nullptr;
const ${record.short_name}_VTable* vtable = nullptr;

/*ctor*/
template<typename T>
${record.short_name}(T& obj) noexcept
    : self(&obj)
    , vtable(${record.short_name}_VTableTraits<T>::get_vtable())
{
}