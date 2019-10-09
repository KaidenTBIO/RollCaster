#ifndef STATE_H
#define STATE_H

class State {
private:
	unsigned char		*m_buffer;
	unsigned int		m_max_size;
	unsigned int		m_size;
	unsigned int		m_ptr;
	
	void		enlarge(unsigned int size);
	
	void		add(const void *data, unsigned int size);
	void		get(void *data, unsigned int size);
	
	void		add_int(int value);
	int		get_int();
	
	void		store_list(const unsigned int *root, unsigned int entry_size);
	void		restore_list(unsigned int *addr, unsigned int entry_size);
	
	void		store_map(const unsigned int *addr, unsigned int entry_size);
	void		restore_map(unsigned int *addr, unsigned int entry_size);
	
	void		store_fpu();
	void		restore_fpu();
public:
	void		store();
	
	void		restore();
	
	void		flush();
	
	bool 		valid();
	
			State();
			~State();
};

#endif
