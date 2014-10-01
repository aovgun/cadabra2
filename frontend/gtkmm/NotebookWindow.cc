
#include <iostream>
#include "NotebookWindow.hh"

cadabra::NotebookWindow::NotebookWindow()
	: b_help(Gtk::Stock::HELP), b_stop(Gtk::Stock::STOP), b_undo(Gtk::Stock::UNDO), b_redo(Gtk::Stock::REDO),
	  modified(false)
	{
	// Connect the dispatcher.
	dispatcher.connect(sigc::mem_fun(*this, &NotebookWindow::process_todo_queue));

	// Setup menu.
	actiongroup=Gtk::ActionGroup::create();
	actiongroup->add( Gtk::Action::create("MenuFile", "_File") );
	actiongroup->add( Gtk::Action::create("MenuEdit", "_Edit") );
	actiongroup->add( Gtk::Action::create("MenuView", "_View") );
	actiongroup->add( Gtk::Action::create("MenuSettings", "_Settings") );
	actiongroup->add( Gtk::Action::create("MenuTutorial", "_Tutorial") );
	actiongroup->add( Gtk::Action::create("MenuFontSize", "Font size") );
	actiongroup->add( Gtk::Action::create("MenuBrainWired", "Brain wired for") );
	actiongroup->add( Gtk::Action::create("MenuHelp", "_Help") );

	add(topbox);
	topbox.pack_start(supermainbox, true, true);
	topbox.pack_start(statusbarbox, false, false);
	supermainbox.pack_start(mainbox, true, true);

	// Status bar
	cdbstatus.set_alignment( 0.0, 0.5 );
	kernelversion.set_alignment( 0.0, 0.5 );
	cdbstatus.set_size_request(200,-1);
	cdbstatus.set_justify(Gtk::JUSTIFY_LEFT);
	kernelversion.set_justify(Gtk::JUSTIFY_LEFT);
	statusbarbox.pack_start(cdbstatus);
	statusbarbox.pack_start(kernelversion);
	statusbarbox.pack_start(progressbar);
	progressbar.set_size_request(200,-1);
	progressbar.set_text("idle");
	progressbar.set_show_text(true);

	// Buttons
	b_stop.set_sensitive(false);
	b_run.set_label("Run all");
	b_run_to.set_label("Run to cursor");
	b_run_from.set_label("Run from cursor");
	b_kill.set_label("Restart kernel");
	buttonbox.pack_start(b_help, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run_to, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_run_from, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_stop, Gtk::PACK_SHRINK);
	buttonbox.pack_start(b_kill, Gtk::PACK_SHRINK);

	// The three main widgets
	mainbox.pack_start(buttonbox, Gtk::PACK_SHRINK, 0);

	set_size_request(800,800);
	update_title();
	show_all();

	}

cadabra::NotebookWindow::~NotebookWindow()
	{
	}

void cadabra::NotebookWindow::set_client(cadabra::Client *cl)
	{
	client=cl;

	// Setup a single-cell document.
	std::lock_guard<std::mutex> guard(client->dtree_mutex);

	cadabra::Client::iterator it=client->dtree().begin();
	auto newcell = std::make_shared<cadabra::Client::DataCell>();
	auto ac = std::make_shared<cadabra::Client::ActionAddCell>(newcell, 
																				  it, 
																				  cadabra::Client::ActionAddCell::Position::child);

	client->perform(ac);
	}

void cadabra::NotebookWindow::update_title()
	{
	if(name.size()>0) {
		if(modified)
			set_title("Cadabra: "+name+"*");
		else
			set_title("Cadabra: "+name);
		}
	else {
		if(modified) 
			set_title("Cadabra*");
		else
			set_title("Cadabra");
		}
	}

void cadabra::NotebookWindow::new_todo_notification() 
	{
	std::cout << "notified" << std::endl;
	dispatcher.emit();
	}

void cadabra::NotebookWindow::process_todo_queue() 
	{
	std::cout << "processing todo queue" << std::endl;
	std::lock_guard<std::mutex> guard(gui_todo_mutex);
	while(gui_todo_deque.size()>0) {
		std::cout << "need to process an action" << std::endl;
		gui_todo_deque.pop_front();
		}
	}

void cadabra::NotebookWindow::on_connect()
	{
	// HERE: use locking and a 'todo' stack to tell the gui what has changed.

	dispatcher.emit();

	// FIXME: this member is called from the network thread; what we should do
	// instead is lock the string with the status, then use the glib dispatcher
	// to inform the gui thread that something has changed.
	kernelversion.set_text("connected");
	}

void cadabra::NotebookWindow::on_disconnect()
	{
	dispatcher.emit();

	kernelversion.set_text("not connected");
	}

void cadabra::NotebookWindow::on_network_error()
	{
	}


//void cadabra::before_tree_change(cadabra::Client::ActionBase ab)
//	{
//	std::lock_guard<std::mutex> guard(todo_mutex);
//	}