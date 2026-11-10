use gtk4::glib;
use gtk4::prelude::*;
use gtk4::subclass::prelude::*;
use std::cell::RefCell;

use crate::headerbuttons::{HeaderButtonsControl, HeaderButtonsHandler};
use crate::impl_header_buttons_control;
use crate::window_bindings;
use super::entry_list_bindings as entry_list;

impl_header_buttons_control!(EntryBrowser, imp::EntryBrowser);

mod imp {
    use super::*;

    #[derive(Default, gtk4::CompositeTemplate)]
    #[template(resource = "/com/bjareholt/johan/simple-diary/ui/entry_browser.ui")]
    pub struct EntryBrowser {
        #[template_child]
        pub split_view: TemplateChild<adw::NavigationSplitView>,
        #[template_child]
        pub entry_list_box: TemplateChild<gtk4::Box>,
        #[template_child]
        pub content_box: TemplateChild<gtk4::Box>,

        pub entry_list: RefCell<Option<gtk4::Widget>>,
        pub content_box_child: RefCell<Option<gtk4::Widget>>,

        /* from header_buttons_control */
        pub back_button: RefCell<Option<gtk4::Button>>,
        pub new_button: RefCell<Option<gtk4::Button>>,
    }

    #[glib::object_subclass]
    impl ObjectSubclass for EntryBrowser {
        const NAME: &'static str = "EntryBrowser";
        type Type = super::EntryBrowser;
        type ParentType = adw::BreakpointBin;
        type Interfaces = (HeaderButtonsControl,);

        fn class_init(klass: &mut Self::Class) {
            klass.bind_template();
        }

        fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
            obj.init_template();
        }
    }

    impl ObjectImpl for EntryBrowser {
        fn constructed(&self) {
            self.parent_constructed();

            let obj = self.obj();

            let list = entry_list::new();

            let obj_weak = obj.downgrade();
            list.clone().upcast::<glib::Object>().connect_closure(
                "selection-changed",
                false,
                glib::closure_local!(
                    move |_list: glib::Object, entry: Option<crate::entry::Entry>| {
                        if let Some(obj) = obj_weak.upgrade() {
                            obj.entry_selected_changed(entry);
                        }
                    }
                ),
            );

            self.entry_list_box.append(&list);
            self.entry_list.replace(Some(list));
        }
    }

    impl WidgetImpl for EntryBrowser {}
    impl adw::subclass::bin::BinImpl for EntryBrowser {}
    impl adw::subclass::breakpoint_bin::BreakpointBinImpl for EntryBrowser {}
}

glib::wrapper! {
    pub struct EntryBrowser(ObjectSubclass<imp::EntryBrowser>)
        @extends adw::BreakpointBin, adw::Bin, gtk4::Widget,
        @implements HeaderButtonsControl, gtk4::ConstraintTarget, gtk4::Buildable, gtk4::Accessible;
}

impl EntryBrowser {
    pub fn new() -> Self {
        glib::Object::new()
    }

    fn entry_list(&self) -> gtk4::Widget {
        self.imp().entry_list.borrow().clone().expect("entry list not initialized")
    }

    fn update_header_buttons(&self) {
        let imp = self.imp();
        let has_content = imp.content_box_child.borrow().is_some();

        if let Some(back_button) = imp.back_button.borrow().as_ref() {
            back_button.set_visible(has_content);
        }
        if let Some(new_button) = imp.new_button.borrow().as_ref() {
            new_button.set_visible(!has_content);
        }
    }

    fn set_content(&self, widget: Option<gtk4::Widget>) {
        let imp = self.imp();

        // Remove old content
        if let Some(old_child) = imp.content_box_child.borrow().as_ref() {
            imp.content_box.remove(old_child);
        }

        // Add new content if there is any
        if let Some(widget) = &widget {
            imp.content_box.append(widget);
            imp.split_view.set_show_content(true);
        } else {
            imp.split_view.set_show_content(false);
        }
        imp.content_box_child.replace(widget);

        self.update_header_buttons();
    }

    fn connect_entry_view_deleted(&self, entry_view: &crate::widgets::entry_view::EntryView) {
        let self_weak = self.downgrade();
        entry_view.connect_closure(
            "deleted",
            false,
            glib::closure_local!(
                move |_view: crate::widgets::entry_view::EntryView, entry: crate::entry::Entry| {
                    if let Some(this) = self_weak.upgrade() {
                        this.entry_view_deleted(&entry);
                    }
                }
            ),
        );
    }

    fn entry_view_deleted(&self, entry: &crate::entry::Entry) {
        self.set_content(None);

        let removed = entry_list::remove(&self.entry_list(), entry);
        assert!(removed, "entry was not found in the entry list");
    }

    fn entry_selected_changed(&self, entry: Option<crate::entry::Entry>) {
        match entry {
            None => self.set_content(None),
            Some(entry) => {
                let entry_view = crate::widgets::entry_view::EntryView::new(&entry);
                self.connect_entry_view_deleted(&entry_view);
                self.set_content(Some(entry_view.upcast()));
            }
        }
    }

    fn entry_name_cb(&self, name: String) {
        let filename = format!("{}.md", name);
        let entry_list_widget = self.entry_list();

        let (entry, row) = match entry_list::find(&entry_list_widget, &filename) {
            Some(row) => {
                let listing = row.child().expect("entry list row has no child");
                let entry = entry_list::listing_get_entry(&listing);
                (entry, row)
            }
            None => {
                let folder = crate::utils::get_diary_folder().expect("failed to get diary folder");
                let entry = crate::entry::Entry::new(&folder, &filename);
                entry_list::add_entry(&entry_list_widget, &entry);
                let row = entry_list::find(&entry_list_widget, &filename)
                    .expect("new entry not found in list");
                (entry, row)
            }
        };

        let entry_edit = crate::widgets::entry_edit::EntryEdit::new(&entry);
        let entry_view = crate::widgets::entry_view::EntryView::new(&entry);
        self.connect_entry_view_deleted(&entry_view);

        entry_list::focus(&entry_list_widget, &row);
        window_bindings::push_view(&entry_edit.upcast::<gtk4::Widget>());
    }
}

impl HeaderButtonsHandler for EntryBrowser {
    fn on_enter(
        &self,
        new_button: &gtk4::Button,
        back_button: &gtk4::Button,
        settings_button: &gtk4::Button,
    ) {
        let imp = self.imp();
        imp.new_button.replace(Some(new_button.clone()));
        imp.back_button.replace(Some(back_button.clone()));

        settings_button.set_visible(true);
        self.update_header_buttons();
    }

    fn on_leave(
        &self,
        _new_button: &gtk4::Button,
        _back_button: &gtk4::Button,
        _settings_button: &gtk4::Button,
    ) {
        println!("leave {:?}", self.imp().obj());
    }

    fn on_new_pressed(&self) {
        let dialog = crate::widgets::entry_name_dialog::EntryNameDialog::new();

        let self_weak = self.downgrade();
        dialog.connect_closure(
            "confirmed",
            false,
            glib::closure_local!(
                move |_dialog: crate::widgets::entry_name_dialog::EntryNameDialog, name: String| {
                    if let Some(this) = self_weak.upgrade() {
                        this.entry_name_cb(name);
                    }
                }
            ),
        );

        let window = window_bindings::instance()
            .downcast::<gtk4::Window>()
            .expect("diary window is not a GtkWindow");
        dialog.open(&window);
    }

    fn on_back_pressed(&self) {
        self.set_content(None);
        entry_list::unfocus(&self.entry_list());
    }
}
