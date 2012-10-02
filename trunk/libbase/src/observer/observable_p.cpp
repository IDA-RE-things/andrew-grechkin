#include <libbase/observer_p.hpp>


namespace Base {

	Observable_p::~Observable_p() {
		m_manager->unregister_all(this);
	}

	Observable_p::Observable_p():
		m_manager(get_simple_change_manager()),
		m_changed(false)
	{
	}

	void Observable_p::register_observer(Observer_p * observer) {
		m_manager->register_observer(this, observer);
	}

	void Observable_p::unregister_observer(Observer_p * observer) {
		m_manager->unregister_observer(this, observer);
	}

	void Observable_p::notify_all(const Event & event) const {
		if (m_changed) {
			m_manager->notify(this, event);
			m_changed = false;
		}
	}

	void Observable_p::set_changed(bool changed) const {
		m_changed = changed;
	}

	bool Observable_p::get_changed() const {
		return m_changed;
	}

}
