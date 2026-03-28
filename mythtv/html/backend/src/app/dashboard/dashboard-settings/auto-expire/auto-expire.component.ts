import { AfterViewInit, Component, Input, OnInit, ViewChild } from '@angular/core';
import { NgForm, FormsModule } from '@angular/forms';
import { TranslateService, TranslateModule } from '@ngx-translate/core';
import { MythService } from 'src/app/services/myth.service';
import { ButtonModule } from 'primeng/button';
import { MessageModule } from 'primeng/message';

import { InputNumberModule } from 'primeng/inputnumber';
import { CheckboxModule } from 'primeng/checkbox';
import { SharedModule } from 'primeng/api';
import { CardModule } from 'primeng/card';
import { DashboardSettingsComponent } from '../dashboard-settings.component';
import { SelectModule } from 'primeng/select';

@Component({
    selector: 'app-auto-expire',
    templateUrl: './auto-expire.component.html',
    styleUrls: ['./auto-expire.component.css'],
    imports: [FormsModule, CardModule, SharedModule, SelectModule, CheckboxModule, InputNumberModule, MessageModule, ButtonModule, TranslateModule]
})
export class AutoExpireComponent implements OnInit, AfterViewInit {

    @ViewChild("autoexpire") currentForm!: NgForm;
    @Input() parent!: DashboardSettingsComponent;
    @Input() tabIndex!: number;

    methodList = [
        { Label: 'dashboard.autoexpire.method_oldest', Value: 1 },
        { Label: 'dashboard.autoexpire.method_lowest', Value: 2 },
        { Label: 'dashboard.autoexpire.method_weighted', Value: 3 },
    ];

    successCount = 0;
    errorCount = 0;

    AutoExpireMethod = 1;
    RerecordWatched = false;
    AutoExpireWatchedPriority = false;
    AutoExpireLiveTVMaxAge = 1;
    AutoExpireDayPriority = 3;
    AutoExpireExtraSpace = 1;
    DeletedMaxAge = 0;

    constructor(private mythService: MythService, private translate: TranslateService) {
    }

    ngOnInit(): void {
        this.loadTranslations();
        this.loadValues();
        this.markPristine();
        this.parent.children[this.tabIndex] = this;
    }

    loadTranslations() {
        this.methodList.forEach((entry) =>
            this.translate.get(entry.Label).subscribe(data => entry.Label = data));
    }

    ngAfterViewInit() {
    }

    dirty() {
        return this.currentForm.dirty;
    }

    loadValues() {
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoExpireMethod", Default: "1" })
            .subscribe({
                next: data => this.AutoExpireMethod = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "RerecordWatched", Default: "0" })
            .subscribe({
                next: data => this.RerecordWatched = (data.String == "1"),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoExpireWatchedPriority", Default: "0" })
            .subscribe({
                next: data => this.AutoExpireWatchedPriority = (data.String == "1"),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoExpireLiveTVMaxAge", Default: "1" })
            .subscribe({
                next: data => this.AutoExpireLiveTVMaxAge = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoExpireDayPriority", Default: "3" })
            .subscribe({
                next: data => this.AutoExpireDayPriority = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "AutoExpireExtraSpace", Default: "1" })
            .subscribe({
                next: data => this.AutoExpireExtraSpace = Number(data.String),
                error: () => this.errorCount++
            });
        this.mythService.GetSetting({ HostName: '_GLOBAL_', Key: "DeletedMaxAge", Default: "0" })
            .subscribe({
                next: data => this.DeletedMaxAge = Number(data.String),
                error: () => this.errorCount++
            });
    }

    markPristine() {
        setTimeout(() => {
            this.currentForm.form.markAsPristine();
            this.parent.showDirty();
        }, 100);
    }

    swObserver = {
        next: (x: any) => {
            if (x.bool)
                this.successCount++;
            else {
                this.errorCount++;
                if (this.currentForm)
                    this.currentForm.form.markAsDirty();
            }
        },
        error: (err: any) => {
            console.error(err);
            this.errorCount++;
            if (this.currentForm)
                this.currentForm.form.markAsDirty();
        },
    };

    saveForm() {
        this.successCount = 0;
        this.errorCount = 0;

        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoExpireMethod",
            Value: String(this.AutoExpireMethod)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "RerecordWatched",
            Value: this.RerecordWatched ? "1" : "0"
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoExpireWatchedPriority",
            Value: this.AutoExpireWatchedPriority ? "1" : "0"
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoExpireLiveTVMaxAge",
            Value: String(this.AutoExpireLiveTVMaxAge)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoExpireDayPriority",
            Value: String(this.AutoExpireDayPriority)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "AutoExpireExtraSpace",
            Value: String(this.AutoExpireExtraSpace)
        }).subscribe(this.swObserver);
        this.mythService.PutSetting({
            HostName: '_GLOBAL_', Key: "DeletedMaxAge",
            Value: String(this.DeletedMaxAge)
        }).subscribe(this.swObserver);
    }

}
